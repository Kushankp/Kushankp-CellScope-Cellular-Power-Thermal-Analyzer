# CellScope - Cellular Power & Thermal Analyzer

[![CI](https://github.com/Kushankp/Kushankp-CellScope-Cellular-Power-Thermal-Analyzer/actions/workflows/ci.yml/badge.svg)](https://github.com/Kushankp/Kushankp-CellScope-Cellular-Power-Thermal-Analyzer/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C.svg)](CMakeLists.txt)
[![Python 3.12](https://img.shields.io/badge/Python-3.12-3776AB.svg)](pyproject.toml)

CellScope is a C++20 and Python diagnostic platform for analyzing simulated modem, radio, battery, CPU, and thermal telemetry. It is designed like an internal cellular engineering tool: stream large logs, compute power and thermal KPIs concurrently, detect regressions, generate reports, and export dark-mode dashboards.

## Motivation

Cellular software teams need tooling that can reason across radio state, sleep state, wake reasons, battery current, thermal pressure, and CPU behavior. CellScope demonstrates that workflow with an open-source, reproducible log-analysis platform built around systems engineering fundamentals: streaming I/O, bounded memory, thread ownership, deterministic reduction, and schema-stable reporting.

## Architecture

```text
CSV / JSONL logs
      |
      v
StreamingParser -> bounded record batches -> ThreadPool workers
                                             |-> KPI accumulator
                                             |-> Regression detector
                                             |-> Timeline sampler
      |
      v
AnalysisReport -> JSON / Markdown / HTML / CSV -> Python dashboard
```

The C++ core owns parsing, analysis, regression detection, benchmarks, and CLI execution. The Python package consumes the JSON report contract for interactive visualization.

## Installation

Requirements:

- CMake 3.24+
- C++20 compiler
- Python 3.12 for dashboard workflows

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
python -m pip install -e .
```

## Usage

```bash
./build/cellscope analyze sample_logs/sample.csv -o reports
./build/cellscope report sample_logs/sample.csv -o reports
./build/cellscope stats sample_logs/sample.csv
./build/cellscope validate sample_logs/sample.csv
./build/cellscope version
./build/cellscope generate-sample --rows 1000000 -o sample_logs/million.csv
./build/cellscope benchmark -o benchmark_output
```

Dashboard export:

```bash
cellscope-dashboard reports/analysis.json -o dashboard_output
```

On headless systems, set `MPLBACKEND=Agg` for matplotlib image export.

## Sample Output

```text
records=4
average_current_ma=547
peak_current_ma=1044
average_temperature_c=39.15
sleep_efficiency_percent=50
```

Generated report files:

- `analysis.json`
- `analysis.md`
- `analysis.html`
- `analysis.csv`

## Log Schema

Required CSV columns and JSONL keys:

```text
timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,packet_count,signal_strength_dbm,power_domain
```

Supported network values include `5G`, `LTE`, `WCDMA`, `GSM`, and `WIFI`.

## Benchmarks

`cellscope benchmark` generates 100K, 500K, and 1M row synthetic logs and measures analyzer runtime. Release builds should be used for meaningful numbers.

```bash
./build/cellscope benchmark -o benchmark_output
```

## Testing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
PYTHONPATH=python python -m pytest python/tests
```

## Project Structure

```text
include/cellscope/   Public C++ interfaces
src/                 C++ implementations and CLI
tests/               GoogleTest coverage
python/              Dashboard package and pytest coverage
docs/                MkDocs and engineering documentation
sample_logs/         Realistic sample telemetry
config/              Analyzer profiles
.github/             CI, release workflow, issue and PR templates
```

## Screenshots

Dashboard exports are written to `dashboard_output/` after running `cellscope-dashboard`. The generated HTML and PNG artifacts are intentionally ignored by Git.

## Roadmap

- Baseline-versus-candidate regression comparison
- Memory and CPU utilization reporting in benchmark Markdown output
- Coverage reporting in CI
- Richer dashboard comparison mode
- Domain-specific modules for battery, thermal, power, and radio analysis

## License

MIT
