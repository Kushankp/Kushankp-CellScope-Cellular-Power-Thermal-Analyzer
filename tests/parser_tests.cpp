#include "cellscope/parser/parser.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

TEST(Parser, StreamsCsvAndRecoversFromBadRows) {
  const auto path = std::filesystem::temp_directory_path() / "cellscope_parser_test.csv";
  std::ofstream out(path);
  out << "timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,"
         "packet_count,signal_strength_dbm,power_domain\n";
  out << "2026-07-30T14:22:01.021,1450,39.8,812,3.8,CONNECTED,5G,AWAKE,MODEM_INTERRUPT,125,-91,MODEM\n";
  out << "bad,row\n";
  out.close();

  cellscope::parser::StreamingParser parser;
  std::size_t batches = 0;
  const auto stats = parser.parse_file(path, [&](cellscope::parser::RecordBatch&& batch) {
    ++batches;
    EXPECT_EQ(batch.size(), 1U);
    EXPECT_EQ(batch.front().cpu_frequency_mhz, 1450U);
  });
  EXPECT_EQ(stats.records, 1U);
  EXPECT_EQ(stats.rejected, 1U);
  EXPECT_EQ(batches, 1U);
}
