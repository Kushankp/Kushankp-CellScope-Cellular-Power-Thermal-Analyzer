#pragma once

#include <cstddef>
#include <filesystem>

namespace cellscope::core {

struct AnalyzerConfig {
  std::size_t batch_size{8192};
  std::size_t worker_threads{0};
  std::size_t max_parse_errors{128};
  double power_spike_ma{900.0};
  double thermal_spike_c{45.0};
  double high_cpu_mhz{2200.0};
  double low_signal_dbm{-115.0};
  std::size_t max_findings{10000};
};

AnalyzerConfig load_config(const std::filesystem::path& path);

}  // namespace cellscope::core
