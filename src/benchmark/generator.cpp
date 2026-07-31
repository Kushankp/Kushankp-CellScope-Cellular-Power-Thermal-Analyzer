#include "cellscope/benchmark/generator.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "cellscope/analysis/analyzer.hpp"

namespace cellscope::benchmark {

void generate_csv_log(const std::filesystem::path& path, std::uint64_t rows) {
  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path());
  }
  std::ofstream out(path);
  out << "timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,"
         "packet_count,signal_strength_dbm,power_domain\n";
  std::mt19937 rng(42);
  std::uniform_int_distribution<int> cpu(300, 2600);
  std::uniform_real_distribution<double> temp(30.0, 48.0);
  std::uniform_real_distribution<double> current(90.0, 1200.0);
  std::uniform_int_distribution<int> rssi(-122, -65);
  const char* radios[] = {"IDLE", "CONNECTED", "SEARCHING", "TRANSMITTING", "RECEIVING"};
  const char* networks[] = {"LTE", "5G", "WCDMA", "GSM"};
  const char* sleeps[] = {"AWAKE", "LIGHT_SLEEP", "DEEP_SLEEP"};
  const char* wakes[] = {"NONE", "MODEM_INTERRUPT", "TIMER", "NETWORK_PAGE", "USER_ACTIVITY"};
  auto base = core::parse_timestamp("2026-07-30T14:22:01.000").value();
  for (std::uint64_t i = 0; i < rows; ++i) {
    auto ts = base + std::chrono::milliseconds(static_cast<int>(i * 10));
    out << core::format_timestamp(ts) << ',' << cpu(rng) << ',' << temp(rng) << ',' << current(rng) << ",3.82,"
        << radios[i % 5] << ',' << networks[i % 4] << ',' << sleeps[i % 3] << ',' << wakes[i % 5] << ','
        << (i * 17) % 5000 << ',' << rssi(rng) << ",MODEM\n";
  }
}

void run_benchmark(const std::filesystem::path& directory) {
  std::filesystem::create_directories(directory);
  for (auto rows : {100000ULL, 500000ULL, 1000000ULL}) {
    const auto path = directory / ("benchmark_" + std::to_string(rows) + ".csv");
    generate_csv_log(path, rows);
    const auto start = std::chrono::steady_clock::now();
    analysis::Analyzer analyzer;
    const auto report = analyzer.analyze(path);
    const auto end = std::chrono::steady_clock::now();
    const auto seconds = std::chrono::duration<double>(end - start).count();
    std::cout << rows << " records: " << seconds << "s, findings=" << report.findings.size() << '\n';
  }
}

}  // namespace cellscope::benchmark
