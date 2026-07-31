#include "cellscope/timeline/timeline.hpp"

#include <algorithm>

namespace cellscope::timeline {

TimelineBuilder::TimelineBuilder(std::size_t max_points) : max_points_(max_points) {}

void TimelineBuilder::add(const core::LogRecord& record) {
  ++seen_;
  if (points_.size() < max_points_ || seen_ % ((seen_ / max_points_) + 1) == 0) {
    if (points_.size() >= max_points_) points_.erase(points_.begin());
    points_.push_back({core::format_timestamp(record.timestamp), record.battery_current_ma, record.temperature_c,
                       static_cast<double>(record.cpu_frequency_mhz),
                       record.wake_reason == core::WakeReason::None ? 0U : 1U,
                       core::to_string(record.radio_state)});
  }
}

void TimelineBuilder::merge(const TimelineBuilder& other) {
  seen_ += other.seen_;
  points_.insert(points_.end(), other.points_.begin(), other.points_.end());
  if (points_.size() > max_points_) {
    points_.erase(points_.begin(), points_.begin() + static_cast<long>(points_.size() - max_points_));
  }
}

std::vector<TimelinePoint> TimelineBuilder::points() const {
  auto out = points_;
  std::ranges::sort(out, {}, &TimelinePoint::timestamp);
  return out;
}

}  // namespace cellscope::timeline
