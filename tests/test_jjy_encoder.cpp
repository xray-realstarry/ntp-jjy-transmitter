#include <cstdio>
#include <string>

#include "jjy_encoder.h"

static int g_failures = 0;

#define CHECK(cond)                                                  \
  do {                                                               \
    if (!(cond)) {                                                   \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);    \
      ++g_failures;                                                  \
    }                                                                \
  } while (0)

static std::string to_string(const jjy::Frame& f) {
  std::string s;
  for (jjy::Symbol sym : f) {
    s += sym == jjy::Symbol::Marker ? 'M' : (sym == jjy::Symbol::One ? '1' : '0');
  }
  return s;
}

static void test_calendar() {
  CHECK(jjy::is_leap_year(2000));
  CHECK(!jjy::is_leap_year(1900));
  CHECK(jjy::is_leap_year(2024));
  CHECK(!jjy::is_leap_year(2026));
  CHECK(jjy::day_of_year(2026, 10, 4) == 277);
  CHECK(jjy::day_of_year(2024, 12, 31) == 366);
  CHECK(jjy::weekday_of(2026, 10, 4) == 0);  // Sunday
  CHECK(jjy::weekday_of(2000, 1, 1) == 6);   // Saturday
}

static void test_from_unix() {
  // 2000-01-01 00:00:00 UTC is 09:00 JST, Saturday.
  jjy::DateTime t = jjy::from_unix(946684800);
  CHECK(t.year == 2000 && t.month == 1 && t.day == 1);
  CHECK(t.hour == 9 && t.minute == 0 && t.weekday == 6);

  // 2026-10-04 03:34:00 UTC is 12:34 JST.
  t = jjy::from_unix(1791084840);
  CHECK(t.year == 2026 && t.month == 10 && t.day == 4);
  CHECK(t.hour == 12 && t.minute == 34 && t.weekday == 0);

  // Local date rolls over relative to UTC: 2026-12-31 15:00 UTC = 2027-01-01 00:00 JST.
  t = jjy::from_unix(1798729200);
  CHECK(t.year == 2027 && t.month == 1 && t.day == 1 && t.hour == 0);

  // Before the epoch (negative values).
  t = jjy::from_unix(-1, 0);
  CHECK(t.year == 1969 && t.month == 12 && t.day == 31);
  CHECK(t.hour == 23 && t.minute == 59);
}

static void test_encode_vector() {
  // Hand-computed vector: 2026-10-04 (Sun, day 277) 12:34 JST.
  const jjy::DateTime t{2026, 10, 4, 12, 34, 0};
  jjy::Frame f;
  CHECK(jjy::encode(t, jjy::LeapSecond::None, f));
  const std::string expected =
      "M"
      "01100100M"
      "000100010M"
      "001000111M"
      "011100010M"
      "000100110M"
      "000000000M";
  CHECK(to_string(f) == expected);
}

static void test_markers_and_parity() {
  for (int hour = 0; hour < 24; ++hour) {
    for (int minute = 0; minute < 60; minute += 7) {
      const jjy::DateTime t{2031, 2, 28, hour, minute,
                            jjy::weekday_of(2031, 2, 28)};
      jjy::Frame f;
      CHECK(jjy::encode(t, jjy::LeapSecond::None, f));
      for (int i : {0, 9, 19, 29, 39, 49, 59}) CHECK(f[i] == jjy::Symbol::Marker);

      int hour_ones = 0, minute_ones = 0;
      for (int i : {12, 13, 15, 16, 17, 18}) hour_ones += f[i] == jjy::Symbol::One;
      for (int i : {1, 2, 3, 5, 6, 7, 8}) minute_ones += f[i] == jjy::Symbol::One;
      CHECK((hour_ones + (f[36] == jjy::Symbol::One)) % 2 == 0);
      CHECK((minute_ones + (f[37] == jjy::Symbol::One)) % 2 == 0);
    }
  }
}

static void test_leap_second_and_invalid() {
  const jjy::DateTime t{2026, 6, 30, 23, 59, 2};
  jjy::Frame f;
  CHECK(jjy::encode(t, jjy::LeapSecond::Insert, f));
  CHECK(f[53] == jjy::Symbol::One && f[54] == jjy::Symbol::One);
  CHECK(jjy::encode(t, jjy::LeapSecond::Delete, f));
  CHECK(f[53] == jjy::Symbol::One && f[54] == jjy::Symbol::Zero);

  CHECK(!jjy::encode({2026, 2, 29, 0, 0, 0}, jjy::LeapSecond::None, f));
  CHECK(!jjy::encode({2026, 13, 1, 0, 0, 0}, jjy::LeapSecond::None, f));
  CHECK(!jjy::encode({2026, 1, 1, 24, 0, 0}, jjy::LeapSecond::None, f));
  CHECK(!jjy::encode({2026, 1, 1, 0, 60, 0}, jjy::LeapSecond::None, f));
}

static void test_pulse_widths() {
  CHECK(jjy::high_ms(jjy::Symbol::Zero) == 800);
  CHECK(jjy::high_ms(jjy::Symbol::One) == 500);
  CHECK(jjy::high_ms(jjy::Symbol::Marker) == 200);
}

int main() {
  test_calendar();
  test_from_unix();
  test_encode_vector();
  test_markers_and_parity();
  test_leap_second_and_invalid();
  test_pulse_widths();
  if (g_failures == 0) std::printf("All tests passed\n");
  return g_failures == 0 ? 0 : 1;
}
