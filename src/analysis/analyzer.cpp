#include "cellscope/analysis/analyzer.hpp"

#include <future>

#include "cellscope/analysis/kpi.hpp"
#include "cellscope/analysis/regression.hpp"
#include "cellscope/parser/parser.hpp"
#include "cellscope/timeline/timeline.hpp"
#include "cellscope/utils/thread_pool.hpp"

namespace cellscope::analysis {

Analyzer::Analyzer(core::AnalyzerConfig config) : config_(config) {}

report::AnalysisReport Analyzer::analyze(const std::filesystem::path& path) const {
  parser::StreamingParser parser(config_);
  utils::ThreadPool pool(config_.worker_threads);
  std::vector<std::future<std::tuple<KpiAccumulator, RegressionDetector, timeline::TimelineBuilder>>> futures;

  auto stats = parser.parse_file(path, [&](parser::RecordBatch&& batch) {
    futures.push_back(pool.submit([batch = std::move(batch), config = config_] {
      KpiAccumulator kpi;
      RegressionDetector detector(config);
      timeline::TimelineBuilder timeline;
      for (const auto& record : batch) {
        kpi.add(record);
        detector.inspect(record);
        timeline.add(record);
      }
      return std::make_tuple(std::move(kpi), std::move(detector), std::move(timeline));
    }));
  });

  KpiAccumulator merged_kpi;
  RegressionDetector merged_detector(config_);
  timeline::TimelineBuilder merged_timeline;
  for (auto& future : futures) {
    auto [kpi, detector, timeline] = future.get();
    merged_kpi.merge(kpi);
    merged_detector.merge(detector);
    merged_timeline.merge(timeline);
  }

  report::AnalysisReport out;
  out.parse_stats = std::move(stats);
  out.kpis = merged_kpi.summary();
  out.findings = merged_detector.findings();
  out.timeline = merged_timeline.points();
  return out;
}

}  // namespace cellscope::analysis
