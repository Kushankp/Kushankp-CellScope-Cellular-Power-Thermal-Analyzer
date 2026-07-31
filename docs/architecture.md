# Architecture

CellScope follows clean architecture boundaries.

```text
CLI -> analysis pipeline -> domain engines
                 |         -> parser
                 |         -> report
                 |         -> timeline
Python dashboard <- JSON report contract
```

The parser streams records into bounded batches. Worker threads compute partial KPI, regression, and timeline state. The analyzer performs a deterministic merge after all futures complete.

Core domain types live under `include/cellscope/core`. Domain engines avoid CLI and visualization dependencies.
