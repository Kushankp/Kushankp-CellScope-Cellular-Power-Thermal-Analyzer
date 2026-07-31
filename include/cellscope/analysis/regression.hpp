#pragma once

#include <string>
#include <vector>

#include "cellscope/core/config.hpp"
#include "cellscope/core/types.hpp"

namespace cellscope::analysis {

enum class Severity { Info, Warning, Critical };

struct RegressionFinding {
  std::string category;
  Severity severity{Severity::Info};
  std::string message;
  core::TimePoint timestamp{};
  double observed{};
  double threshold{};
};

std::string to_string(Severity severity);

class RegressionDetector {
 public:
  explicit RegressionDetector(core::AnalyzerConfig config = {});
  void inspect(const core::LogRecord& record);
  void merge(const RegressionDetector& other);
  const std::vector<RegressionFinding>& findings() const noexcept { return findings_; }

 private:
  core::AnalyzerConfig config_;
  std::vector<RegressionFinding> findings_;
};

}  // namespace cellscope::analysis
