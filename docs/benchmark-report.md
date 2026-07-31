# Benchmark Report

The benchmark command generates synthetic logs at 100K, 500K, and 1M rows, then measures analyzer wall-clock time.

```bash
cellscope benchmark -o benchmark_output
```

Results vary by CPU, storage, compiler, and build type. Use Release builds for meaningful measurements.
