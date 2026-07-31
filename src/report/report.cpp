#include "cellscope/report/report.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace cellscope::report {
namespace {

std::string esc(const std::string& value) {
  std::string out;
  for (char c : value) {
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else out += c;
  }
  return out;
}

template <class Map>
void json_map(std::ostringstream& os, const Map& map) {
  os << "{";
  bool first = true;
  for (const auto& [key, value] : map) {
    if (!first) os << ",";
    first = false;
    os << "\"" << esc(key) << "\":" << value;
  }
  os << "}";
}

}  // namespace

std::string to_json(const AnalysisReport& report) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(3);
  os << "{\n\"parse_stats\":{\"records\":" << report.parse_stats.records
     << ",\"rejected\":" << report.parse_stats.rejected << "},\n";
  os << "\"kpis\":{";
  os << "\"records\":" << report.kpis.records;
  os << ",\"average_current_ma\":" << report.kpis.average_current_ma;
  os << ",\"peak_current_ma\":" << report.kpis.peak_current_ma;
  os << ",\"estimated_battery_drain_mah\":" << report.kpis.estimated_battery_drain_mah;
  os << ",\"average_temperature_c\":" << report.kpis.average_temperature_c;
  os << ",\"peak_temperature_c\":" << report.kpis.peak_temperature_c;
  os << ",\"average_cpu_mhz\":" << report.kpis.average_cpu_mhz;
  os << ",\"wake_events\":" << report.kpis.wake_events;
  os << ",\"wake_frequency_per_hour\":" << report.kpis.wake_frequency_per_hour;
  os << ",\"radio_connected_percent\":" << report.kpis.radio_connected_percent;
  os << ",\"sleep_efficiency_percent\":" << report.kpis.sleep_efficiency_percent;
  os << ",\"radio_state_counts\":";
  json_map(os, report.kpis.radio_state_counts);
  os << ",\"network_counts\":";
  json_map(os, report.kpis.network_counts);
  os << "},\n\"findings\":[";
  for (std::size_t i = 0; i < report.findings.size(); ++i) {
    const auto& f = report.findings[i];
    if (i != 0) os << ",";
    os << "{\"category\":\"" << esc(f.category) << "\",\"severity\":\"" << analysis::to_string(f.severity)
       << "\",\"message\":\"" << esc(f.message) << "\",\"timestamp\":\"" << core::format_timestamp(f.timestamp)
       << "\",\"observed\":" << f.observed << ",\"threshold\":" << f.threshold << "}";
  }
  os << "],\n\"timeline\":[";
  for (std::size_t i = 0; i < report.timeline.size(); ++i) {
    const auto& p = report.timeline[i];
    if (i != 0) os << ",";
    os << "{\"timestamp\":\"" << esc(p.timestamp) << "\",\"power_ma\":" << p.power_ma
       << ",\"temperature_c\":" << p.temperature_c << ",\"cpu_mhz\":" << p.cpu_mhz
       << ",\"wake_event\":" << p.wake_event << ",\"radio_state\":\"" << esc(p.radio_state) << "\"}";
  }
  os << "]\n}\n";
  return os.str();
}

std::string to_markdown(const AnalysisReport& report) {
  std::ostringstream os;
  os << "# CellScope Analysis Report\n\n";
  os << "## Summary\n\n";
  os << "- Records: " << report.kpis.records << "\n";
  os << "- Parse rejects: " << report.parse_stats.rejected << "\n";
  os << "- Average current: " << report.kpis.average_current_ma << " mA\n";
  os << "- Peak current: " << report.kpis.peak_current_ma << " mA\n";
  os << "- Estimated drain: " << report.kpis.estimated_battery_drain_mah << " mAh\n";
  os << "- Average temperature: " << report.kpis.average_temperature_c << " C\n";
  os << "- Peak temperature: " << report.kpis.peak_temperature_c << " C\n";
  os << "- Sleep efficiency: " << report.kpis.sleep_efficiency_percent << "%\n\n";
  os << "## Findings\n\n";
  for (const auto& f : report.findings) {
    os << "- **" << analysis::to_string(f.severity) << "** `" << f.category << "` "
       << core::format_timestamp(f.timestamp) << ": " << f.message << " (" << f.observed << " >= "
       << f.threshold << ")\n";
  }
  return os.str();
}

std::string to_html(const AnalysisReport& report) {
  std::ostringstream os;
  os << "<!doctype html><html><head><meta charset=\"utf-8\"><title>CellScope Report</title>"
     << "<style>body{font-family:-apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;margin:32px;"
     << "background:#111;color:#eee}table{border-collapse:collapse}td,th{padding:6px 10px;border:1px solid #444}"
     << ".critical{color:#ff7676}.warning{color:#ffd166}</style></head><body>";
  os << "<h1>CellScope Analysis Report</h1><table><tr><th>KPI</th><th>Value</th></tr>";
  os << "<tr><td>Records</td><td>" << report.kpis.records << "</td></tr>";
  os << "<tr><td>Average Current</td><td>" << report.kpis.average_current_ma << " mA</td></tr>";
  os << "<tr><td>Peak Temperature</td><td>" << report.kpis.peak_temperature_c << " C</td></tr>";
  os << "<tr><td>Sleep Efficiency</td><td>" << report.kpis.sleep_efficiency_percent << "%</td></tr>";
  os << "</table><h2>Findings</h2><ul>";
  for (const auto& f : report.findings) {
    os << "<li class=\"" << analysis::to_string(f.severity) << "\">" << f.category << ": " << f.message
       << " at " << core::format_timestamp(f.timestamp) << "</li>";
  }
  os << "</ul></body></html>\n";
  return os.str();
}

std::string to_csv(const AnalysisReport& report) {
  std::ostringstream os;
  os << "metric,value\n";
  os << "records," << report.kpis.records << '\n';
  os << "parse_rejected," << report.parse_stats.rejected << '\n';
  os << "average_current_ma," << report.kpis.average_current_ma << '\n';
  os << "peak_current_ma," << report.kpis.peak_current_ma << '\n';
  os << "estimated_battery_drain_mah," << report.kpis.estimated_battery_drain_mah << '\n';
  os << "average_temperature_c," << report.kpis.average_temperature_c << '\n';
  os << "peak_temperature_c," << report.kpis.peak_temperature_c << '\n';
  os << "average_cpu_mhz," << report.kpis.average_cpu_mhz << '\n';
  os << "wake_events," << report.kpis.wake_events << '\n';
  os << "wake_frequency_per_hour," << report.kpis.wake_frequency_per_hour << '\n';
  os << "radio_connected_percent," << report.kpis.radio_connected_percent << '\n';
  os << "sleep_efficiency_percent," << report.kpis.sleep_efficiency_percent << '\n';
  os << "\nfinding_category,severity,timestamp,message,observed,threshold\n";
  for (const auto& f : report.findings) {
    os << f.category << ',' << analysis::to_string(f.severity) << ',' << core::format_timestamp(f.timestamp)
       << ",\"" << esc(f.message) << "\"," << f.observed << ',' << f.threshold << '\n';
  }
  return os.str();
}

void write_report_files(const AnalysisReport& report, const std::filesystem::path& output_dir) {
  std::filesystem::create_directories(output_dir);
  std::ofstream(output_dir / "analysis.json") << to_json(report);
  std::ofstream(output_dir / "analysis.md") << to_markdown(report);
  std::ofstream(output_dir / "analysis.html") << to_html(report);
  std::ofstream(output_dir / "analysis.csv") << to_csv(report);
}

}  // namespace cellscope::report
