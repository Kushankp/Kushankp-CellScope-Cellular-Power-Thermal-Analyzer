#include "cellscope/analysis/regression.hpp"

namespace cellscope::analysis {

std::string to_string(Severity severity) {
  switch (severity) {
    case Severity::Warning: return "warning";
    case Severity::Critical: return "critical";
    default: return "info";
  }
}

RegressionDetector::RegressionDetector(core::AnalyzerConfig config) : config_(config) {}

void RegressionDetector::inspect(const core::LogRecord& record) {
  const auto add = [&](std::string category, Severity severity, std::string message, double observed,
                       double threshold) {
    if (findings_.size() < config_.max_findings) {
      findings_.push_back({std::move(category), severity, std::move(message), record.timestamp, observed, threshold});
    }
  };
  if (record.battery_current_ma >= config_.power_spike_ma) {
    add("power", Severity::Critical, "battery current exceeded spike threshold", record.battery_current_ma,
        config_.power_spike_ma);
  }
  if (record.temperature_c >= config_.thermal_spike_c) {
    add("thermal", Severity::Critical, "temperature exceeded thermal spike threshold", record.temperature_c,
        config_.thermal_spike_c);
  }
  if (record.cpu_frequency_mhz >= config_.high_cpu_mhz) {
    add("cpu", Severity::Warning, "CPU frequency remained in high-performance range", record.cpu_frequency_mhz,
        config_.high_cpu_mhz);
  }
  if (record.wake_reason == core::WakeReason::ModemInterrupt &&
      record.sleep_state != core::SleepState::Awake) {
    add("wake", Severity::Warning, "modem interrupt woke a sleeping system", 1.0, 1.0);
  }
  if (record.signal_strength_dbm <= config_.low_signal_dbm) {
    add("network", Severity::Warning, "signal strength below configured quality floor",
        static_cast<double>(record.signal_strength_dbm), config_.low_signal_dbm);
  }
}

void RegressionDetector::merge(const RegressionDetector& other) {
  findings_.insert(findings_.end(), other.findings_.begin(), other.findings_.end());
}

}  // namespace cellscope::analysis
