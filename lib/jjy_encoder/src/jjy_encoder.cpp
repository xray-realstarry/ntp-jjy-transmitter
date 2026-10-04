#include "jjy_encoder.h"

namespace jjy {

namespace {

struct Bit {
  int pos;     // second within the frame
  int weight;  // value represented by this bit
};

// Weights are BCD-like, so a greedy decomposition yields the transmitted bits.
constexpr Bit kMinute[] = {{1, 40}, {2, 20}, {3, 10}, {5, 8}, {6, 4}, {7, 2}, {8, 1}};
constexpr Bit kHour[] = {{12, 20}, {13, 10}, {15, 8}, {16, 4}, {17, 2}, {18, 1}};
constexpr Bit kDayOfYear[] = {{22, 200}, {23, 100}, {25, 80}, {26, 40}, {27, 20},
                              {28, 10},  {30, 8},   {31, 4},  {32, 2},  {33, 1}};
constexpr Bit kYear[] = {{41, 80}, {42, 40}, {43, 20}, {44, 10},
                         {45, 8},  {46, 4},  {47, 2},  {48, 1}};
constexpr Bit kWeekday[] = {{50, 4}, {51, 2}, {52, 1}};

constexpr int kMarkers[] = {0, 9, 19, 29, 39, 49, 59};
constexpr int kPosPA1 = 36;  // even parity of the hour bits
constexpr int kPosPA2 = 37;  // even parity of the minute bits
constexpr int kPosLS1 = 53;
constexpr int kPosLS2 = 54;

template <std::size_t N>
void put_value(Frame& f, const Bit (&bits)[N], int value) {
  for (const Bit& b : bits) {
    if (value >= b.weight) {
      f[b.pos] = Symbol::One;
      value -= b.weight;
    }
  }
}

template <std::size_t N>
Symbol parity_of(const Frame& f, const Bit (&bits)[N]) {
  int ones = 0;
  for (const Bit& b : bits) {
    if (f[b.pos] == Symbol::One) ++ones;
  }
  return (ones % 2) ? Symbol::One : Symbol::Zero;
}

int days_in_month(int year, int month) {
  static constexpr int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && is_leap_year(year)) return 29;
  return kDays[month - 1];
}

}  // namespace

bool is_leap_year(int year) {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int day_of_year(int year, int month, int day) {
  int n = day;
  for (int m = 1; m < month; ++m) n += days_in_month(year, m);
  return n;
}

int weekday_of(int year, int month, int day) {
  // Sakamoto's algorithm; 0 = Sunday.
  static constexpr int kOffset[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) --year;
  return (year + year / 4 - year / 100 + year / 400 + kOffset[month - 1] + day) % 7;
}

bool is_valid(const DateTime& t) {
  if (t.year < 0 || t.month < 1 || t.month > 12) return false;
  if (t.day < 1 || t.day > days_in_month(t.year, t.month)) return false;
  if (t.hour < 0 || t.hour > 23 || t.minute < 0 || t.minute > 59) return false;
  return t.weekday >= 0 && t.weekday <= 6;
}

DateTime from_unix(std::int64_t unix_seconds, int utc_offset_seconds) {
  const std::int64_t local = unix_seconds + utc_offset_seconds;
  std::int64_t days = local / 86400;
  std::int64_t rem = local % 86400;
  if (rem < 0) {
    rem += 86400;
    --days;
  }

  // Civil-from-days (H. Hinnant), epoch 1970-01-01.
  const std::int64_t z = days + 719468;
  const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
  const std::int64_t doe = z - era * 146097;
  const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const std::int64_t mp = (5 * doy + 2) / 153;
  const int day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
  const int month = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
  const int year = static_cast<int>(yoe + era * 400 + (month <= 2 ? 1 : 0));

  DateTime t;
  t.year = year;
  t.month = month;
  t.day = day;
  t.hour = static_cast<int>(rem / 3600);
  t.minute = static_cast<int>((rem % 3600) / 60);
  t.weekday = weekday_of(year, month, day);
  return t;
}

bool encode(const DateTime& t, LeapSecond leap, Frame& out) {
  if (!is_valid(t)) return false;

  out.fill(Symbol::Zero);
  for (int pos : kMarkers) out[pos] = Symbol::Marker;

  put_value(out, kMinute, t.minute);
  put_value(out, kHour, t.hour);
  put_value(out, kDayOfYear, day_of_year(t.year, t.month, t.day));
  put_value(out, kYear, t.year % 100);
  put_value(out, kWeekday, t.weekday);

  out[kPosPA1] = parity_of(out, kHour);
  out[kPosPA2] = parity_of(out, kMinute);

  if (leap != LeapSecond::None) {
    out[kPosLS1] = Symbol::One;
    if (leap == LeapSecond::Insert) out[kPosLS2] = Symbol::One;
  }
  return true;
}

std::uint16_t high_ms(Symbol s) {
  switch (s) {
    case Symbol::Zero:
      return 800;
    case Symbol::One:
      return 500;
    case Symbol::Marker:
      return 200;
  }
  return 0;
}

}  // namespace jjy
