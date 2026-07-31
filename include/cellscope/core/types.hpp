#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cellscope::core {

using TimePoint = std::chrono::sys_time<std::chrono::milliseconds>;

enum class RadioState { Unknown, Idle, Connected, Searching, Transmitting, Receiving };
enum class NetworkType { Unknown, LTE, NR5G, WCDMA, GSM, WiFi };
enum class SleepState { Unknown, Awake, LightSleep, DeepSleep };
enum class WakeReason { None, ModemInterrupt, Timer, NetworkPage, UserActivity, Thermal, Unknown };

struct LogRecord {
  TimePoint timestamp{};
  std::uint32_t cpu_frequency_mhz{};
  double temperature_c{};
  double battery_current_ma{};
  double voltage_v{};
  RadioState radio_state{RadioState::Unknown};
  NetworkType network_type{NetworkType::Unknown};
  SleepState sleep_state{SleepState::Unknown};
  WakeReason wake_reason{WakeReason::Unknown};
  std::uint64_t packet_count{};
  int signal_strength_dbm{};
  std::string power_domain;
};

struct ParseError {
  std::uint64_t line{};
  std::string message;
  std::string raw;
};

struct ParseStats {
  std::uint64_t records{};
  std::uint64_t rejected{};
  std::vector<ParseError> errors;
};

std::string to_string(RadioState value);
std::string to_string(NetworkType value);
std::string to_string(SleepState value);
std::string to_string(WakeReason value);

RadioState parse_radio_state(std::string_view value);
NetworkType parse_network_type(std::string_view value);
SleepState parse_sleep_state(std::string_view value);
WakeReason parse_wake_reason(std::string_view value);

std::optional<TimePoint> parse_timestamp(std::string_view value);
std::string format_timestamp(TimePoint value);

}  // namespace cellscope::core
