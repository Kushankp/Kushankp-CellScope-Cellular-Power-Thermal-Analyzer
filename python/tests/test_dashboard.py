from pathlib import Path

from cellscope.dashboard import build_interactive_dashboard, load_report, timeline_frame


def test_dashboard_loads_report(tmp_path: Path) -> None:
    report_path = tmp_path / "analysis.json"
    report_path.write_text(
        '{"timeline":[{"timestamp":"2026-07-30T14:22:01.021","power_ma":812,'
        '"temperature_c":39.8,"cpu_mhz":1450,"wake_event":1,"radio_state":"CONNECTED"}]}',
        encoding="utf-8",
    )
    report = load_report(report_path)
    frame = timeline_frame(report)
    fig = build_interactive_dashboard(report)
    assert len(frame) == 1
    assert len(fig.data) == 3
