#include "cellscope/analysis/analyzer.hpp"

#include <deque>
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
  using PartialResult = std::tuple<KpiAccumulator, RegressionDetector, timeline::TimelineBuilder>;
  std::deque<std::future<PartialResult>> futures;
  KpiAccumulator merged_kpi;
  RegressionDetector merged_detector(config_);
  timeline::TimelineBuilder merged_timeline;

  const auto max_in_flight = std::max<std::size_t>(1, pool.size() * 2);
  const auto drain_one = [&] {
    auto [kpi, detector, timeline] = futures.front().get();
    futures.pop_front();
    merged_kpi.merge(kpi);
    merged_detector.merge(detector);
    merged_timeline.merge(timeline);
  };

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
    if (futures.size() >= max_in_flight) {
      drain_one();
    }
  });

  while (!futures.empty()) {
    drain_one();
  }

  report::AnalysisReport out;
  out.parse_stats = std::move(stats);
  out.kpis = merged_kpi.summary();
  out.findings = merged_detector.findings();
  out.timeline = merged_timeline.points();
  return out;
}

}  // namespace cellscope::analysis
