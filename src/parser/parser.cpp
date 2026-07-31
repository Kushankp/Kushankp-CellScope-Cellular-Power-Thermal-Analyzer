#include "cellscope/parser/parser.hpp"

#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

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

std::unordered_map<std::string, std::string> json_object(const std::string& line) {
  std::unordered_map<std::string, std::string> values;
  std::string body = line;
  if (!body.empty() && body.front() == '{') body.erase(body.begin());
  if (!body.empty() && body.back() == '}') body.pop_back();
  for (const auto& token : split_csv(body)) {
    const auto pos = token.find(':');
    if (pos == std::string::npos) continue;
    values.emplace(trim(token.substr(0, pos)), trim(token.substr(pos + 1)));
  }
  return values;
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
  return record;
}

core::LogRecord parse_csv_record(const std::vector<std::string>& header, const std::string& line) {
  const auto fields = split_csv(line);
  if (fields.size() != header.size()) throw std::runtime_error("field count mismatch");
  std::unordered_map<std::string, std::string> values;
  for (std::size_t i = 0; i < header.size(); ++i) values.emplace(header[i], fields[i]);
  return from_map(values);
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
      auto record = format == InputFormat::Csv ? parse_csv_record(header, line) : from_map(json_object(line));
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
