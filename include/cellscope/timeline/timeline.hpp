#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cellscope/core/types.hpp"

namespace cellscope::timeline {

struct TimelinePoint {
  std::string timestamp;
  double power_ma{};
  double temperature_c{};
  double cpu_mhz{};
  std::uint64_t wake_event{};
  std::string radio_state;
};

class TimelineBuilder {
 public:
  explicit TimelineBuilder(std::size_t max_points = 2000);
  void add(const core::LogRecord& record);
  void merge(const TimelineBuilder& other);
  std::vector<TimelinePoint> points() const;

 private:
  std::size_t max_points_;
  std::uint64_t seen_{};
  std::vector<TimelinePoint> points_;
};

}  // namespace cellscope::timeline
