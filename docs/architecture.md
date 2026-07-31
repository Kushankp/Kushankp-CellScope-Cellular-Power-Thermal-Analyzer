# Architecture

CellScope follows clean architecture boundaries. CLI, filesystem, and report presentation sit at the edge; parsing, analysis, regression detection, and timeline reduction live in the C++ core.

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

The parser streams records into bounded batches. Worker threads compute partial KPI, regression, and timeline state. The analyzer caps in-flight worker futures and performs deterministic reduction as batches complete.

Core domain types live under `include/cellscope/core`. Domain engines avoid CLI and visualization dependencies.
