#include "cellscope/analysis/regression.hpp"

#include <gtest/gtest.h>

namespace {

cellscope::core::LogRecord anomalous_record() {
  cellscope::core::LogRecord record;
  record.timestamp = cellscope::core::parse_timestamp("2026-07-30T14:22:01.000").value();
  record.cpu_frequency_mhz = 2600;
  record.temperature_c = 50.0;
  record.battery_current_ma = 1200.0;
  record.voltage_v = 3.8;
  record.radio_state = cellscope::core::RadioState::Connected;
  record.network_type = cellscope::core::NetworkType::NR5G;
  record.sleep_state = cellscope::core::SleepState::LightSleep;
  record.wake_reason = cellscope::core::WakeReason::ModemInterrupt;
  record.packet_count = 128;
  record.signal_strength_dbm = -120;
  record.power_domain = "MODEM";
  return record;
}

}  // namespace

TEST(RegressionDetector, CapsRetainedFindings) {
  cellscope::core::AnalyzerConfig config;
  config.max_findings = 3;
  cellscope::analysis::RegressionDetector detector(config);
  const auto record = anomalous_record();

  detector.inspect(record);
  detector.inspect(record);

  EXPECT_EQ(detector.findings().size(), 3U);
}
