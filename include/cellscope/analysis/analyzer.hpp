#pragma once

#include <filesystem>

#include "cellscope/core/config.hpp"
#include "cellscope/report/report.hpp"

namespace cellscope::analysis {

class Analyzer {
 public:
  explicit Analyzer(core::AnalyzerConfig config = {});
  report::AnalysisReport analyze(const std::filesystem::path& path) const;

 private:
  core::AnalyzerConfig config_;
};

}  // namespace cellscope::analysis
