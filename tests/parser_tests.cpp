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

TEST(Parser, StreamsJsonLinesAndRecoversFromMalformedObjects) {
  const auto path = std::filesystem::temp_directory_path() / "cellscope_parser_test.jsonl";
  std::ofstream out(path);
  out << R"({"timestamp":"2026-07-30T14:22:01.021","cpu_mhz":1450,"temperature_c":39.8,)"
         R"("current_ma":812,"voltage_v":3.8,"radio_state":"CONNECTED","network_type":"5G",)"
         R"("sleep_state":"AWAKE","wake_reason":"MODEM_INTERRUPT","packet_count":125,)"
         R"("signal_strength_dbm":-91,"power_domain":"MODEM"})"
      << '\n';
  out << R"({"timestamp":"2026-07-30T14:22:02.021","cpu_mhz":"fast"})" << '\n';
  out << "{not-json}\n";
  out.close();

  cellscope::parser::StreamingParser parser;
  std::size_t records = 0;
  const auto stats = parser.parse_file(path, [&](cellscope::parser::RecordBatch&& batch) {
    records += batch.size();
    EXPECT_EQ(batch.front().network_type, cellscope::core::NetworkType::NR5G);
  });
  EXPECT_EQ(records, 1U);
  EXPECT_EQ(stats.records, 1U);
  EXPECT_EQ(stats.rejected, 2U);
}

TEST(Parser, RejectsInvalidEnumValues) {
  const auto path = std::filesystem::temp_directory_path() / "cellscope_parser_invalid_enum.csv";
  std::ofstream out(path);
  out << "timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,"
         "packet_count,signal_strength_dbm,power_domain\n";
  out << "2026-07-30T14:22:01.021,1450,39.8,812,3.8,CONNECTED,SATELLITE,AWAKE,NONE,125,-91,MODEM\n";
  out.close();

  cellscope::parser::StreamingParser parser;
  const auto stats = parser.parse_file(path, [](cellscope::parser::RecordBatch&&) {});
  EXPECT_EQ(stats.records, 0U);
  EXPECT_EQ(stats.rejected, 1U);
}
