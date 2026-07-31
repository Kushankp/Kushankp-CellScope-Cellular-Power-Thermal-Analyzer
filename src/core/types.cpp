#include "cellscope/core/types.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <sstream>

namespace cellscope::core {
namespace {

std::string upper(std::string_view value) {
  std::string out(value);
  std::ranges::transform(out, out.begin(), [](unsigned char c) { return std::toupper(c); });
  return out;
}

}  // namespace

std::string to_string(RadioState value) {
  switch (value) {
    case RadioState::Idle: return "IDLE";
    case RadioState::Connected: return "CONNECTED";
    case RadioState::Searching: return "SEARCHING";
    case RadioState::Transmitting: return "TRANSMITTING";
    case RadioState::Receiving: return "RECEIVING";
    default: return "UNKNOWN";
  }
}

std::string to_string(NetworkType value) {
  switch (value) {
    case NetworkType::LTE: return "LTE";
    case NetworkType::NR5G: return "5G";
    case NetworkType::WCDMA: return "WCDMA";
    case NetworkType::GSM: return "GSM";
    case NetworkType::WiFi: return "WIFI";
    default: return "UNKNOWN";
  }
}

std::string to_string(SleepState value) {
  switch (value) {
    case SleepState::Awake: return "AWAKE";
    case SleepState::LightSleep: return "LIGHT_SLEEP";
    case SleepState::DeepSleep: return "DEEP_SLEEP";
    default: return "UNKNOWN";
  }
}

std::string to_string(WakeReason value) {
  switch (value) {
    case WakeReason::None: return "NONE";
    case WakeReason::ModemInterrupt: return "MODEM_INTERRUPT";
    case WakeReason::Timer: return "TIMER";
    case WakeReason::NetworkPage: return "NETWORK_PAGE";
    case WakeReason::UserActivity: return "USER_ACTIVITY";
    case WakeReason::Thermal: return "THERMAL";
    default: return "UNKNOWN";
  }
}

RadioState parse_radio_state(std::string_view value) {
  const auto v = upper(value);
  if (v == "IDLE") return RadioState::Idle;
  if (v == "CONNECTED") return RadioState::Connected;
  if (v == "SEARCHING") return RadioState::Searching;
  if (v == "TRANSMITTING" || v == "TX") return RadioState::Transmitting;
  if (v == "RECEIVING" || v == "RX") return RadioState::Receiving;
  return RadioState::Unknown;
}

NetworkType parse_network_type(std::string_view value) {
  const auto v = upper(value);
  if (v == "LTE") return NetworkType::LTE;
  if (v == "5G" || v == "NR" || v == "NR5G") return NetworkType::NR5G;
  if (v == "WCDMA") return NetworkType::WCDMA;
  if (v == "GSM") return NetworkType::GSM;
  if (v == "WIFI" || v == "WI-FI") return NetworkType::WiFi;
  return NetworkType::Unknown;
}

SleepState parse_sleep_state(std::string_view value) {
  const auto v = upper(value);
  if (v == "AWAKE") return SleepState::Awake;
  if (v == "LIGHT_SLEEP" || v == "LIGHT") return SleepState::LightSleep;
  if (v == "DEEP_SLEEP" || v == "DEEP") return SleepState::DeepSleep;
  return SleepState::Unknown;
}

WakeReason parse_wake_reason(std::string_view value) {
  const auto v = upper(value);
  if (v.empty() || v == "NONE") return WakeReason::None;
  if (v == "MODEM_INTERRUPT") return WakeReason::ModemInterrupt;
  if (v == "TIMER") return WakeReason::Timer;
  if (v == "NETWORK_PAGE") return WakeReason::NetworkPage;
  if (v == "USER_ACTIVITY") return WakeReason::UserActivity;
  if (v == "THERMAL") return WakeReason::Thermal;
  return WakeReason::Unknown;
}

std::optional<TimePoint> parse_timestamp(std::string_view value) {
  if (value.size() < 19) return std::nullopt;
  std::tm tm{};
  std::istringstream ss(std::string(value.substr(0, 19)));
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  if (ss.fail()) return std::nullopt;
  auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::sys_days{std::chrono::year{tm.tm_year + 1900} /
                            std::chrono::month{static_cast<unsigned>(tm.tm_mon + 1)} /
                            std::chrono::day{static_cast<unsigned>(tm.tm_mday)}} +
      std::chrono::hours{tm.tm_hour} + std::chrono::minutes{tm.tm_min} +
      std::chrono::seconds{tm.tm_sec});
  if (value.size() > 20 && value[19] == '.') {
    int millis = 0;
    auto ms = value.substr(20, 3);
    std::from_chars(ms.data(), ms.data() + ms.size(), millis);
    tp += std::chrono::milliseconds{millis};
  }
  return std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
}

std::string format_timestamp(TimePoint value) {
  const auto days = std::chrono::floor<std::chrono::days>(value);
  const std::chrono::year_month_day ymd{days};
  const auto tod = value - days;
  const auto h = std::chrono::duration_cast<std::chrono::hours>(tod);
  const auto m = std::chrono::duration_cast<std::chrono::minutes>(tod - h);
  const auto s = std::chrono::duration_cast<std::chrono::seconds>(tod - h - m);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tod - h - m - s);
  std::ostringstream os;
  os << static_cast<int>(ymd.year()) << '-' << std::setw(2) << std::setfill('0')
     << static_cast<unsigned>(ymd.month()) << '-' << std::setw(2)
     << static_cast<unsigned>(ymd.day()) << 'T' << std::setw(2) << h.count() << ':'
     << std::setw(2) << m.count() << ':' << std::setw(2) << s.count() << '.' << std::setw(3)
     << ms.count();
  return os.str();
}

}  // namespace cellscope::core
