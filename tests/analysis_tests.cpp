#include "cellscope/analysis/analyzer.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

TEST(Analysis, ComputesKpisAndFindings) {
  const auto path = std::filesystem::temp_directory_path() / "cellscope_analysis_test.csv";
  std::ofstream out(path);
  out << "timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,"
         "packet_count,signal_strength_dbm,power_domain\n";
  out << "2026-07-30T14:00:00.000,1000,35,100,3.8,IDLE,LTE,DEEP_SLEEP,NONE,0,-80,AP\n";
  out << "2026-07-30T15:00:00.000,2400,50,1000,3.8,CONNECTED,5G,LIGHT_SLEEP,MODEM_INTERRUPT,100,-120,MODEM\n";
  out.close();

  cellscope::core::AnalyzerConfig config;
  config.batch_size = 1;
  config.worker_threads = 2;
  cellscope::analysis::Analyzer analyzer(config);
  const auto report = analyzer.analyze(path);

  EXPECT_EQ(report.kpis.records, 2U);
  EXPECT_DOUBLE_EQ(report.kpis.average_current_ma, 550.0);
  EXPECT_DOUBLE_EQ(report.kpis.peak_temperature_c, 50.0);
  EXPECT_GE(report.findings.size(), 4U);
}
