# Developer Guide

Keep new behavior behind the public headers in `include/cellscope`. Add focused GoogleTest coverage for C++ changes and pytest coverage for Python dashboard changes.

Run before submitting:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
python -m pytest
```
