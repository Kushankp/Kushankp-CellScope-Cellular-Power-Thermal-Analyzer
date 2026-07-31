# User Guide

Use `cellscope analyze` for full report generation and `cellscope stats` for quick terminal summaries.

Configuration profiles are YAML files with thresholds such as `power_spike_ma`, `thermal_spike_c`, and `high_cpu_mhz`.

Malformed records are skipped and counted. A bounded set of parse errors is retained for diagnostics.
