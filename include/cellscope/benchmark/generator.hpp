#pragma once

#include <cstdint>
#include <filesystem>

namespace cellscope::benchmark {

void generate_csv_log(const std::filesystem::path& path, std::uint64_t rows);
void run_benchmark(const std::filesystem::path& directory);

}  // namespace cellscope::benchmark
