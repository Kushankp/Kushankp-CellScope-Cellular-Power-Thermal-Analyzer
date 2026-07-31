#include "cellscope/core/config.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace cellscope::core {
namespace {

void trim_in_place(std::string& value) {
  const auto first = value.find_first_not_of(" \t");
  if (first == std::string::npos) {
    value.clear();
    return;
  }
  const auto last = value.find_last_not_of(" \t");
  value = value.substr(first, last - first + 1);
}

}  // namespace

AnalyzerConfig load_config(const std::filesystem::path& path) {
  AnalyzerConfig config;
  std::ifstream in(path);
  if (!in) return config;
  std::string line;
  while (std::getline(in, line)) {
    const auto pos = line.find(':');
    if (pos == std::string::npos) continue;
    auto key = line.substr(0, pos);
    auto value = line.substr(pos + 1);
    trim_in_place(key);
    trim_in_place(value);
    std::istringstream ss(value);
    if (key == "batch_size") ss >> config.batch_size;
    if (key == "worker_threads") ss >> config.worker_threads;
    if (key == "max_parse_errors") ss >> config.max_parse_errors;
    if (key == "power_spike_ma") ss >> config.power_spike_ma;
    if (key == "thermal_spike_c") ss >> config.thermal_spike_c;
    if (key == "high_cpu_mhz") ss >> config.high_cpu_mhz;
    if (key == "low_signal_dbm") ss >> config.low_signal_dbm;
    if (key == "max_findings") ss >> config.max_findings;
  }
  return config;
}

}  // namespace cellscope::core
