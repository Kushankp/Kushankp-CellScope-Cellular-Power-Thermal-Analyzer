#include <CLI/CLI.hpp>

#include <filesystem>
#include <iostream>

#include <spdlog/spdlog.h>

#include "cellscope/analysis/analyzer.hpp"
#include "cellscope/benchmark/generator.hpp"
#include "cellscope/core/config.hpp"
#include "cellscope/report/report.hpp"

int main(int argc, char** argv) {
  CLI::App app{"CellScope - Cellular Power & Thermal Analyzer"};
  std::filesystem::path config_path;
  app.add_option("--config", config_path, "YAML configuration profile");

  std::filesystem::path analyze_input;
  std::filesystem::path analyze_output{"reports"};
  auto* analyze = app.add_subcommand("analyze", "Analyze a CSV or JSONL modem log");
  analyze->add_option("log", analyze_input, "Input log path")->required();
  analyze->add_option("-o,--output", analyze_output, "Output directory");

  std::filesystem::path stats_input;
  auto* stats = app.add_subcommand("stats", "Print KPI summary for a log");
  stats->add_option("log", stats_input, "Input log path")->required();

  std::filesystem::path report_input;
  std::filesystem::path report_output{"reports"};
  auto* report_cmd = app.add_subcommand("report", "Generate HTML, Markdown, JSON, and CSV reports for a log");
  report_cmd->add_option("log", report_input, "Input log path")->required();
  report_cmd->add_option("-o,--output", report_output, "Output directory");

  std::filesystem::path generate_output{"sample_logs/generated.csv"};
  std::uint64_t generate_rows = 10000;
  auto* generate = app.add_subcommand("generate-sample", "Generate a realistic synthetic CSV log");
  generate->add_option("-o,--output", generate_output, "Output CSV path");
  generate->add_option("--rows", generate_rows, "Number of rows");

  std::filesystem::path benchmark_dir{"benchmark_output"};
  auto* benchmark = app.add_subcommand("benchmark", "Run 100K, 500K, and 1M record benchmarks");
  benchmark->add_option("-o,--output", benchmark_dir, "Benchmark working directory");

  CLI11_PARSE(app, argc, argv);

  try {
    auto config = config_path.empty() ? cellscope::core::AnalyzerConfig{} : cellscope::core::load_config(config_path);
    if (*analyze) {
      cellscope::analysis::Analyzer analyzer(config);
      const auto report = analyzer.analyze(analyze_input);
      cellscope::report::write_report_files(report, analyze_output);
      spdlog::info("analyzed {} records with {} rejected rows", report.parse_stats.records,
                   report.parse_stats.rejected);
      std::cout << cellscope::report::to_markdown(report);
    } else if (*stats) {
      cellscope::analysis::Analyzer analyzer(config);
      const auto report = analyzer.analyze(stats_input);
      std::cout << "records=" << report.kpis.records << '\n'
                << "average_current_ma=" << report.kpis.average_current_ma << '\n'
                << "peak_current_ma=" << report.kpis.peak_current_ma << '\n'
                << "average_temperature_c=" << report.kpis.average_temperature_c << '\n'
                << "sleep_efficiency_percent=" << report.kpis.sleep_efficiency_percent << '\n';
    } else if (*report_cmd) {
      cellscope::analysis::Analyzer analyzer(config);
      const auto report = analyzer.analyze(report_input);
      cellscope::report::write_report_files(report, report_output);
      spdlog::info("wrote reports for {} records to {}", report.parse_stats.records, report_output.string());
    } else if (*generate) {
      cellscope::benchmark::generate_csv_log(generate_output, generate_rows);
      spdlog::info("generated {} rows at {}", generate_rows, generate_output.string());
    } else if (*benchmark) {
      cellscope::benchmark::run_benchmark(benchmark_dir);
    } else {
      std::cout << app.help();
    }
  } catch (const std::exception& ex) {
    spdlog::error("{}", ex.what());
    return 1;
  }
  return 0;
}
