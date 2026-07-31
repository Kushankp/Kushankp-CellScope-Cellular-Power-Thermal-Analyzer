#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "cellscope/core/types.hpp"

namespace cellscope::analysis {

struct KpiSummary {
  std::uint64_t records{};
  double average_current_ma{};
  double peak_current_ma{};
  double estimated_battery_drain_mah{};
  double average_temperature_c{};
  double peak_temperature_c{};
  double average_cpu_mhz{};
  std::uint64_t wake_events{};
  double wake_frequency_per_hour{};
  double radio_connected_percent{};
  double sleep_efficiency_percent{};
  std::map<std::string, std::uint64_t> radio_state_counts;
  std::map<std::string, std::uint64_t> network_counts;
};

class KpiAccumulator {
 public:
  void add(const core::LogRecord& record);
  void merge(const KpiAccumulator& other);
  KpiSummary summary() const;

 private:
  std::uint64_t records_{};
  double current_sum_{};
  double peak_current_{};
  double temp_sum_{};
  double peak_temp_{};
  double cpu_sum_{};
  std::uint64_t wake_events_{};
  std::uint64_t radio_connected_{};
  std::uint64_t sleep_records_{};
  core::TimePoint first_{};
  core::TimePoint last_{};
  bool has_time_{false};
  std::map<std::string, std::uint64_t> radio_counts_;
  std::map<std::string, std::uint64_t> network_counts_;
};

}  // namespace cellscope::analysis
