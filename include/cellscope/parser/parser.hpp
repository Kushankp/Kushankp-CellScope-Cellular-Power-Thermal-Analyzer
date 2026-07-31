#pragma once

#include <filesystem>
#include <functional>
#include <vector>

#include "cellscope/core/config.hpp"
#include "cellscope/core/types.hpp"

namespace cellscope::parser {

using RecordBatch = std::vector<core::LogRecord>;
using BatchConsumer = std::function<void(RecordBatch&&)>;

enum class InputFormat { Csv, JsonLines };

InputFormat detect_format(const std::filesystem::path& path);

class StreamingParser {
 public:
  explicit StreamingParser(core::AnalyzerConfig config = {});
  core::ParseStats parse_file(const std::filesystem::path& path, BatchConsumer consumer) const;

 private:
  core::AnalyzerConfig config_;
};

}  // namespace cellscope::parser
