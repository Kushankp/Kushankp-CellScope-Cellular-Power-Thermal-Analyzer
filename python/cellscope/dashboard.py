from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

import matplotlib.pyplot as plt
import pandas as pd
import plotly.express as px
from plotly.subplots import make_subplots
import plotly.graph_objects as go


def load_report(path: str | Path) -> dict[str, Any]:
    with Path(path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def timeline_frame(report: dict[str, Any]) -> pd.DataFrame:
    frame = pd.DataFrame(report.get("timeline", []))
    if not frame.empty:
        frame["timestamp"] = pd.to_datetime(frame["timestamp"])
    return frame


def build_interactive_dashboard(report: dict[str, Any]) -> go.Figure:
    frame = timeline_frame(report)
    fig = make_subplots(
        rows=3,
        cols=1,
        shared_xaxes=True,
        subplot_titles=("Battery Current", "Temperature", "CPU Frequency"),
    )
    if not frame.empty:
        fig.add_trace(go.Scatter(x=frame["timestamp"], y=frame["power_ma"], name="Current mA"), row=1, col=1)
        fig.add_trace(go.Scatter(x=frame["timestamp"], y=frame["temperature_c"], name="Temperature C"), row=2, col=1)
        fig.add_trace(go.Scatter(x=frame["timestamp"], y=frame["cpu_mhz"], name="CPU MHz"), row=3, col=1)
    fig.update_layout(template="plotly_dark", title="CellScope Power & Thermal Timeline", height=900)
    return fig


def export_dashboard(report_path: str | Path, output_dir: str | Path) -> None:
    report = load_report(report_path)
    output = Path(output_dir)
    output.mkdir(parents=True, exist_ok=True)

    fig = build_interactive_dashboard(report)
    fig.write_html(output / "dashboard.html")

    frame = timeline_frame(report)
    if not frame.empty:
        plt.style.use("dark_background")
        ax = frame.plot(x="timestamp", y=["power_ma", "temperature_c", "cpu_mhz"], figsize=(12, 7))
        ax.set_title("CellScope Timeline")
        ax.figure.tight_layout()
        ax.figure.savefig(output / "timeline.png", dpi=160)
        plt.close(ax.figure)

        heatmap = px.density_heatmap(frame, x="radio_state", y="temperature_c", z="power_ma", template="plotly_dark")
        heatmap.write_html(output / "radio_thermal_heatmap.html")


def main() -> None:
    parser = argparse.ArgumentParser(description="Export CellScope dashboard artifacts")
    parser.add_argument("report", type=Path)
    parser.add_argument("-o", "--output", type=Path, default=Path("dashboard_output"))
    args = parser.parse_args()
    export_dashboard(args.report, args.output)


if __name__ == "__main__":
    main()
