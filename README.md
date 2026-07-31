# CellScope - Cellular Power & Thermal Analyzer

CellScope is a C++20 and Python diagnostic platform for analyzing simulated modem, radio, power, and thermal logs. It is designed like an internal cellular engineering tool: streaming parsers, concurrent KPI calculation, regression detection, timeline generation, reports, benchmarks, and a dark-mode visualization dashboard.

## Features

- Streaming CSV and JSONL parser with validation and error recovery
- Multithreaded KPI and regression analysis
- Power, thermal, battery, CPU, wake, radio, network, and sleep-efficiency metrics
- HTML, Markdown, and JSON reports
- Synthetic modem log generator and 100K/500K/1M benchmark runner
- Python Plotly/matplotlib dashboard exports
- CMake, GoogleTest, pytest, clang-format, clang-tidy, GitHub Actions, Doxygen, and MkDocs scaffolding

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Usage

```bash
./build/cellscope analyze sample_logs/sample.csv -o reports
./build/cellscope stats sample_logs/sample.csv
./build/cellscope generate-sample --rows 1000000 -o sample_logs/million.csv
./build/cellscope benchmark -o benchmark_output
```

Dashboard:

```bash
python -m pip install -e .
cellscope-dashboard reports/analysis.json -o dashboard_output
```

## Log Schema

Required CSV columns:

```text
timestamp,cpu_mhz,temperature_c,current_ma,voltage_v,radio_state,network_type,sleep_state,wake_reason,packet_count,signal_strength_dbm,power_domain
```

JSONL uses the same field names per object.

## License

MIT
