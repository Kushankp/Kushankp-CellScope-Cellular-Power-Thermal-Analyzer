#include "cellscope/analysis/kpi.hpp"

#include <algorithm>

namespace cellscope::analysis {

void KpiAccumulator::add(const core::LogRecord& record) {
  if (!has_time_) {
    first_ = record.timestamp;
    last_ = record.timestamp;
    has_time_ = true;
  } else {
    first_ = std::min(first_, record.timestamp);
    last_ = std::max(last_, record.timestamp);
  }
  ++records_;
  current_sum_ += record.battery_current_ma;
  peak_current_ = std::max(peak_current_, record.battery_current_ma);
  temp_sum_ += record.temperature_c;
  peak_temp_ = std::max(peak_temp_, record.temperature_c);
  cpu_sum_ += record.cpu_frequency_mhz;
  if (record.wake_reason != core::WakeReason::None) ++wake_events_;
  if (record.radio_state == core::RadioState::Connected || record.radio_state == core::RadioState::Transmitting ||
      record.radio_state == core::RadioState::Receiving) {
    ++radio_connected_;
  }
  if (record.sleep_state == core::SleepState::LightSleep || record.sleep_state == core::SleepState::DeepSleep) {
    ++sleep_records_;
  }
  ++radio_counts_[core::to_string(record.radio_state)];
  ++network_counts_[core::to_string(record.network_type)];
}

void KpiAccumulator::merge(const KpiAccumulator& other) {
  if (other.records_ == 0) return;
  if (!has_time_) {
    first_ = other.first_;
    last_ = other.last_;
    has_time_ = true;
  } else {
    first_ = std::min(first_, other.first_);
    last_ = std::max(last_, other.last_);
  }
  records_ += other.records_;
  current_sum_ += other.current_sum_;
  peak_current_ = std::max(peak_current_, other.peak_current_);
  temp_sum_ += other.temp_sum_;
  peak_temp_ = std::max(peak_temp_, other.peak_temp_);
  cpu_sum_ += other.cpu_sum_;
  wake_events_ += other.wake_events_;
  radio_connected_ += other.radio_connected_;
  sleep_records_ += other.sleep_records_;
  for (const auto& [key, value] : other.radio_counts_) radio_counts_[key] += value;
  for (const auto& [key, value] : other.network_counts_) network_counts_[key] += value;
}

KpiSummary KpiAccumulator::summary() const {
  KpiSummary out;
  out.records = records_;
  if (records_ == 0) return out;
  const auto hours = has_time_ ? std::max(1.0 / 3600.0,
                                         std::chrono::duration<double>(last_ - first_).count() / 3600.0)
                               : 1.0;
  out.average_current_ma = current_sum_ / static_cast<double>(records_);
  out.peak_current_ma = peak_current_;
  out.estimated_battery_drain_mah = out.average_current_ma * hours;
  out.average_temperature_c = temp_sum_ / static_cast<double>(records_);
  out.peak_temperature_c = peak_temp_;
  out.average_cpu_mhz = cpu_sum_ / static_cast<double>(records_);
  out.wake_events = wake_events_;
  out.wake_frequency_per_hour = static_cast<double>(wake_events_) / hours;
  out.radio_connected_percent = 100.0 * static_cast<double>(radio_connected_) / static_cast<double>(records_);
  out.sleep_efficiency_percent = 100.0 * static_cast<double>(sleep_records_) / static_cast<double>(records_);
  out.radio_state_counts = radio_counts_;
  out.network_counts = network_counts_;
  return out;
}

}  // namespace cellscope::analysis
