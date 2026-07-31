"""CellScope Python dashboard package."""

from typing import Any

__all__ = ["load_report"]


def __getattr__(name: str) -> Any:
    if name == "load_report":
        from .dashboard import load_report

        return load_report
    raise AttributeError(name)
