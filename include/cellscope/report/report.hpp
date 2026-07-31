#pragma once

#include <filesystem>

#include "cellscope/analysis/kpi.hpp"
#include "cellscope/analysis/regression.hpp"
#include "cellscope/core/types.hpp"
#include "cellscope/timeline/timeline.hpp"

namespace cellscope::report {

struct AnalysisReport {
  core::ParseStats parse_stats;
  analysis::KpiSummary kpis;
  std::vector<analysis::RegressionFinding> findings;
  std::vector<timeline::TimelinePoint> timeline;
};

std::string to_json(const AnalysisReport& report);
std::string to_markdown(const AnalysisReport& report);
std::string to_html(const AnalysisReport& report);
std::string to_csv(const AnalysisReport& report);
void write_report_files(const AnalysisReport& report, const std::filesystem::path& output_dir);

}  // namespace cellscope::report
