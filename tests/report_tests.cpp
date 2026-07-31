#include "cellscope/report/report.hpp"

#include <gtest/gtest.h>

TEST(Report, SerializesJsonAndMarkdown) {
  cellscope::report::AnalysisReport report;
  report.kpis.records = 7;
  report.kpis.average_current_ma = 123.4;
  const auto json = cellscope::report::to_json(report);
  const auto md = cellscope::report::to_markdown(report);
  EXPECT_NE(json.find("\"records\":7"), std::string::npos);
  EXPECT_NE(md.find("CellScope Analysis Report"), std::string::npos);
}
