#pragma once

#include <array>
#include <cstdint>

// Pure C++ JJY time code encoder. No Arduino/ESP-IDF dependency so that it can
// be unit-tested on the host.
namespace jjy {

constexpr int kFrameLength = 60;

enum class Symbol : std::uint8_t { Zero, One, Marker };

// Leap second notice carried in LS1/LS2 of the frame.
// NOTE: the LS1/LS2 encoding is not yet verified against the NICT spec.
enum class LeapSecond : std::uint8_t { None, Insert, Delete };

struct DateTime {
  int year;     // full year, e.g. 2026 (only the last two digits are sent)
  int month;    // 1-12
  int day;      // 1-31
  int hour;     // 0-23
  int minute;   // 0-59
  int weekday;  // 0 = Sunday ... 6 = Saturday
};

using Frame = std::array<Symbol, kFrameLength>;

bool is_leap_year(int year);
int day_of_year(int year, int month, int day);
int weekday_of(int year, int month, int day);
bool is_valid(const DateTime& t);

// Converts a Unix time to a calendar time with the given UTC offset
// (default: JST, which is what JJY transmits). Does not depend on the C
// library time zone.
DateTime from_unix(std::int64_t unix_seconds, int utc_offset_seconds = 9 * 3600);

// Builds the 60-symbol frame for the minute that starts at `t` (second 0 of
// the frame is the start of that minute). Returns false if `t` is invalid.
bool encode(const DateTime& t, LeapSecond leap, Frame& out);

// Duration of the full-amplitude part at the start of a second, in ms.
// The carrier is reduced for the rest of the second.
std::uint16_t high_ms(Symbol s);

}  // namespace jjy
