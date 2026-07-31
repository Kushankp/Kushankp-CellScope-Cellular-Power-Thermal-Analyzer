#include "cellscope/parser/parser.hpp"

#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace cellscope::parser {
namespace {

std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n\"");
  if (first == std::string::npos) return {};
  const auto last = value.find_last_not_of(" \t\r\n\"");
  return value.substr(first, last - first + 1);
}

std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> fields;
  std::string field;
  bool quoted = false;
  for (char c : line) {
    if (c == '"') {
      quoted = !quoted;
    } else if (c == ',' && !quoted) {
      fields.push_back(trim(field));
      field.clear();
    } else {
      field.push_back(c);
    }
  }
  fields.push_back(trim(field));
  return fields;
}

double parse_double(const std::string& value, const char* suffix = nullptr) {
  std::string normalized = trim(value);
  if (suffix != nullptr) {
    if (const auto pos = normalized.find(suffix); pos != std::string::npos) {
      normalized.erase(pos);
    }
  }
  double out = 0.0;
  auto [ptr, ec] = std::from_chars(normalized.data(), normalized.data() + normalized.size(), out);
  if (ec != std::errc()) throw std::runtime_error("invalid number: " + value);
  return out;
}

std::uint64_t parse_u64(const std::string& value) {
  std::uint64_t out = 0;
  auto v = trim(value);
  auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), out);
  if (ec != std::errc()) throw std::runtime_error("invalid integer: " + value);
  return out;
}

core::LogRecord from_map(const std::unordered_map<std::string, std::string>& values) {
  auto get = [&](const std::string& key) -> const std::string& {
    const auto it = values.find(key);
    if (it == values.end()) throw std::runtime_error("missing field: " + key);
    return it->second;
  };
  core::LogRecord record;
  auto ts = core::parse_timestamp(get("timestamp"));
  if (!ts) throw std::runtime_error("invalid timestamp");
  record.timestamp = *ts;
  record.cpu_frequency_mhz = static_cast<std::uint32_t>(parse_u64(get("cpu_mhz")));
  record.temperature_c = parse_double(get("temperature_c"), "C");
  record.battery_current_ma = parse_double(get("current_ma"), "mA");
  record.voltage_v = parse_double(get("voltage_v"), "V");
  record.radio_state = core::parse_radio_state(get("radio_state"));
  record.network_type = core::parse_network_type(get("network_type"));
  record.sleep_state = core::parse_sleep_state(get("sleep_state"));
  record.wake_reason = core::parse_wake_reason(get("wake_reason"));
  record.packet_count = parse_u64(get("packet_count"));
  record.signal_strength_dbm = static_cast<int>(parse_double(get("signal_strength_dbm")));
  record.power_domain = get("power_domain");
  if (record.radio_state == core::RadioState::Unknown) throw std::runtime_error("invalid radio_state");
  if (record.network_type == core::NetworkType::Unknown) throw std::runtime_error("invalid network_type");
  if (record.sleep_state == core::SleepState::Unknown) throw std::runtime_error("invalid sleep_state");
  if (record.wake_reason == core::WakeReason::Unknown) throw std::runtime_error("invalid wake_reason");
  if (record.voltage_v <= 0.0) throw std::runtime_error("voltage_v must be positive");
  if (record.temperature_c < -40.0 || record.temperature_c > 125.0) {
    throw std::runtime_error("temperature_c outside supported range");
  }
  return record;
}

core::LogRecord parse_csv_record(const std::vector<std::string>& header, const std::string& line) {
  const auto fields = split_csv(line);
  if (fields.size() != header.size()) throw std::runtime_error("field count mismatch");
  std::unordered_map<std::string, std::string> values;
  for (std::size_t i = 0; i < header.size(); ++i) values.emplace(header[i], fields[i]);
  return from_map(values);
}

core::LogRecord parse_json_record(const std::string& line) {
  const auto json = nlohmann::json::parse(line);
  auto require_string = [&](const char* key) {
    if (!json.contains(key)) throw std::runtime_error(std::string("missing field: ") + key);
    if (!json.at(key).is_string()) throw std::runtime_error(std::string("field must be string: ") + key);
    return json.at(key).get<std::string>();
  };
  auto require_number = [&](const char* key) {
    if (!json.contains(key)) throw std::runtime_error(std::string("missing field: ") + key);
    if (!json.at(key).is_number()) throw std::runtime_error(std::string("field must be numeric: ") + key);
    return json.at(key).get<double>();
  };

  core::LogRecord record;
  auto ts = core::parse_timestamp(require_string("timestamp"));
  if (!ts) throw std::runtime_error("invalid timestamp");
  record.timestamp = *ts;
  record.cpu_frequency_mhz = static_cast<std::uint32_t>(require_number("cpu_mhz"));
  record.temperature_c = require_number("temperature_c");
  record.battery_current_ma = require_number("current_ma");
  record.voltage_v = require_number("voltage_v");
  record.radio_state = core::parse_radio_state(require_string("radio_state"));
  record.network_type = core::parse_network_type(require_string("network_type"));
  record.sleep_state = core::parse_sleep_state(require_string("sleep_state"));
  record.wake_reason = core::parse_wake_reason(require_string("wake_reason"));
  record.packet_count = static_cast<std::uint64_t>(require_number("packet_count"));
  record.signal_strength_dbm = static_cast<int>(require_number("signal_strength_dbm"));
  record.power_domain = require_string("power_domain");
  if (record.radio_state == core::RadioState::Unknown) throw std::runtime_error("invalid radio_state");
  if (record.network_type == core::NetworkType::Unknown) throw std::runtime_error("invalid network_type");
  if (record.sleep_state == core::SleepState::Unknown) throw std::runtime_error("invalid sleep_state");
  if (record.wake_reason == core::WakeReason::Unknown) throw std::runtime_error("invalid wake_reason");
  if (record.voltage_v <= 0.0) throw std::runtime_error("voltage_v must be positive");
  if (record.temperature_c < -40.0 || record.temperature_c > 125.0) {
    throw std::runtime_error("temperature_c outside supported range");
  }
  return record;
}

}  // namespace

InputFormat detect_format(const std::filesystem::path& path) {
  const auto ext = path.extension().string();
  if (ext == ".json" || ext == ".jsonl") return InputFormat::JsonLines;
  return InputFormat::Csv;
}

StreamingParser::StreamingParser(core::AnalyzerConfig config) : config_(config) {}

core::ParseStats StreamingParser::parse_file(const std::filesystem::path& path,
                                             BatchConsumer consumer) const {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("unable to open input file: " + path.string());

  core::ParseStats stats;
  RecordBatch batch;
  batch.reserve(config_.batch_size);

  const auto flush = [&] {
    if (!batch.empty()) {
      consumer(std::move(batch));
      batch = RecordBatch{};
      batch.reserve(config_.batch_size);
    }
  };

  std::string line;
  std::uint64_t line_no = 0;
  std::vector<std::string> header;
  const auto format = detect_format(path);
  if (format == InputFormat::Csv && std::getline(in, line)) {
    ++line_no;
    header = split_csv(line);
  }

  while (std::getline(in, line)) {
    ++line_no;
    if (line.empty()) continue;
    try {
      auto record = format == InputFormat::Csv ? parse_csv_record(header, line) : parse_json_record(line);
      batch.push_back(std::move(record));
      ++stats.records;
      if (batch.size() >= config_.batch_size) flush();
    } catch (const std::exception& ex) {
      ++stats.rejected;
      if (stats.errors.size() < config_.max_parse_errors) {
        stats.errors.push_back({line_no, ex.what(), line});
      }
    }
  }
  flush();
  return stats;
}

}  // namespace cellscope::parser
