/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformDateFormat.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <limits>

namespace hermes {
namespace platform_unicode {
namespace {

const int64_t SECONDS_PER_DAY = 86400;
const int64_t SECONDS_PER_HOUR = 3600;
const int64_t SECONDS_PER_MINUTE = 60;

/// A date broken into its civil calendar fields.
struct CivilDate {
  int64_t year; ///< May be zero or negative for BCE years.
  unsigned month; ///< 1 through 12.
  unsigned day; ///< 1 through 31.
};

/// Floor division, which unlike C++'s truncating division gives the same
/// answer for negative inputs that the calendar needs.
/// \p rem receives the non-negative remainder.
int64_t floorDiv(int64_t a, int64_t b, int64_t *rem) {
  int64_t q = a / b;
  int64_t r = a % b;
  if (r < 0) {
    --q;
    r += b;
  }
  *rem = r;
  return q;
}

/// \return the civil date \p days after 1970-01-01, which may be negative.
/// This is Howard Hinnant's civil_from_days, which is exact over the whole
/// proleptic Gregorian calendar rather than only the range time_t covers.
CivilDate civilFromDays(int64_t days) {
  days += 719468;
  int64_t era = (days >= 0 ? days : days - 146096) / 146097;
  int64_t doe = days - era * 146097; // [0, 146096]
  int64_t yoe =
      (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
  int64_t y = yoe + era * 400;
  int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
  int64_t mp = (5 * doy + 2) / 153; // [0, 11]
  int64_t d = doy - (153 * mp + 2) / 5 + 1; // [1, 31]
  int64_t m = mp + (mp < 10 ? 3 : -9); // [1, 12]
  return {y + (m <= 2 ? 1 : 0), (unsigned)m, (unsigned)d};
}

/// \return the number of days from 1970-01-01 to the given civil date.
/// The inverse of civilFromDays.
int64_t daysFromCivil(int64_t y, unsigned m, unsigned d) {
  y -= m <= 2;
  int64_t era = (y >= 0 ? y : y - 399) / 400;
  int64_t yoe = y - era * 400; // [0, 399]
  int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
  int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; // [0, 146096]
  return era * 146097 + doe - 719468;
}

/// \return true if \p y is a leap year in the proleptic Gregorian calendar.
bool isLeapYear(int64_t y) {
  return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

/// \return the day of the week for \p days after 1970-01-01, 0 for Sunday.
/// 1970-01-01 was a Thursday.
int weekdayFromDays(int64_t days) {
  int64_t rem;
  floorDiv(days + 4, 7, &rem);
  return (int)rem;
}

/// \return a year in [2000, 2027] with the same leap-year character as \p year
/// and the same weekday for January 1.
///
/// The Gregorian calendar repeats every 28 years inside a span with no skipped
/// leap year, and 2000 through 2027 is such a span because 2000 is divisible
/// by 400. A match therefore always exists. Remapping into this window lets
/// localtime_r be asked about timestamps whose real year it cannot represent.
int64_t equivalentYear(int64_t year) {
  bool leap = isLeapYear(year);
  int weekday = weekdayFromDays(daysFromCivil(year, 1, 1));
  for (int64_t y = 2000; y <= 2027; ++y) {
    if (isLeapYear(y) == leap &&
        weekdayFromDays(daysFromCivil(y, 1, 1)) == weekday)
      return y;
  }
  assert(false && "no equivalent year in a 28-year Gregorian cycle");
  return 2001;
}

/// \return whether \p epochMs, which falls in the civil year \p year, must be
/// remapped onto an equivalent year before it is passed to localtime_r
/// (localtime_s on Windows).
///
/// Outside Windows, this matches the native range of LocalTimeOffsetCache
/// (kMinNativeTimeMs and kMaxNativeTimeMs in hermes/VM/JSLib/DateCache.h), so
/// that toLocaleString() and the Date getters pass the same times to the OS
/// unchanged. With a 32-bit time_t, both remap the times outside that range,
/// but onto different equivalent years, so their offsets can differ there.
/// That header is not included here because lib/Platform must not depend on
/// lib/VM.
bool needsEquivalentYear(int64_t epochMs, int64_t year) {
#ifdef _WINDOWS
  // MSVC's localtime_s rejects a negative time_t outright. The upper bound
  // keeps the original window, which ends 18 days before
  // LocalTimeOffsetCache's Windows limit of 2^31-1 s.
  (void)epochMs;
  return year < 1970 || year > 2037;
#else
  (void)year;
  // A 64-bit time_t represents every Date value, and localtime_r then follows
  // the timezone's history and current rules.
  if (sizeof(time_t) >= sizeof(int64_t))
    return false;
  // Compare milliseconds, as LocalTimeOffsetCache does.
  constexpr int64_t maxTimeMs =
      (int64_t)std::numeric_limits<int32_t>::max() * 1000;
  return epochMs < -maxTimeMs || epochMs > maxTimeMs;
#endif
}

/// \return the host's UTC offset in seconds at \p epochMs, positive east of
/// Greenwich.
///
/// Times for which needsEquivalentYear() returns true are remapped onto an
/// equivalent year first, which preserves the weekday and leap-year character
/// and therefore the daylight-saving transitions the host would apply.
int64_t localOffsetSeconds(int64_t epochMs) {
  int64_t msOfSecond;
  int64_t epochSecs = floorDiv(epochMs, 1000, &msOfSecond);
  int64_t secsOfDay;
  int64_t days = floorDiv(epochSecs, SECONDS_PER_DAY, &secsOfDay);
  CivilDate civil = civilFromDays(days);

  // Remapping trades accuracy for range: the host applies its rules for the
  // remapped (2000-2027) date, not the history tzdata records for the real
  // year, so a zone whose rules changed over time can be off versus ICU.
  // TZ=EST+5, with no DST at any date, hides this.
  int64_t probeSecs = epochSecs;
  if (needsEquivalentYear(epochMs, civil.year)) {
    int64_t eqYear = equivalentYear(civil.year);
    probeSecs =
        daysFromCivil(eqYear, civil.month, civil.day) * SECONDS_PER_DAY +
        secsOfDay;
  }

  time_t t = (time_t)probeSecs;
  struct tm local;
#ifdef _WINDOWS
  if (localtime_s(&local, &t) != 0)
    return 0;
#else
  if (localtime_r(&t, &local) == nullptr)
    return 0;
#endif

  // Re-encode the broken-down local time as if it were UTC, then subtract the
  // instant it came from. The difference is the offset. This avoids relying on
  // tm_gmtoff, which is not portable.
  int64_t localAsUTC =
      daysFromCivil(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday) *
          SECONDS_PER_DAY +
      local.tm_hour * SECONDS_PER_HOUR + local.tm_min * SECONDS_PER_MINUTE +
      local.tm_sec;
  return localAsUTC - probeSecs;
}

/// Append the ASCII \p str to \p buf.
void append(llvh::SmallVectorImpl<char16_t> &buf, const char *str) {
  for (const char *p = str; *p != '\0'; ++p)
    buf.push_back((char16_t)*p);
}

/// Append \p value in decimal, zero-padded to at least \p width digits.
void appendNumber(
    llvh::SmallVectorImpl<char16_t> &buf,
    int64_t value,
    int width) {
  char tmp[32];
  std::snprintf(tmp, sizeof(tmp), "%0*lld", width, (long long)value);
  append(buf, tmp);
}

} // namespace

void formatDateTimeFixed(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  assert(std::isfinite(unixtimeMs) && "caller must reject non-finite times");
  buf.clear();
  if (!formatDate && !formatTime)
    return;

  // Floor rather than truncate, so times before the epoch do not round up.
  int64_t utcSecs = (int64_t)std::floor(unixtimeMs / 1000.0);
  int64_t localSecs =
      utcSecs + localOffsetSeconds((int64_t)std::floor(unixtimeMs));

  int64_t secsOfDay;
  int64_t days = floorDiv(localSecs, SECONDS_PER_DAY, &secsOfDay);
  CivilDate civil = civilFromDays(days);

  if (formatDate) {
    static const char *const kMonths[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"};
    append(buf, kMonths[civil.month - 1]);
    buf.push_back(u' ');
    appendNumber(buf, (int64_t)civil.day, 1);
    append(buf, ", ");
    // CLDR's `y` field has no minimum width, so ICU renders year 500 as
    // "500". Padding to four here would print "0500" and break the
    // byte-identical match this formatter exists to preserve. A negative
    // year keeps its sign; see doc/SpecIncompat.md.
    appendNumber(buf, civil.year, 1);
  }

  if (formatDate && formatTime)
    append(buf, ", ");

  if (formatTime) {
    int64_t hour24 = secsOfDay / SECONDS_PER_HOUR;
    int64_t minute = (secsOfDay / SECONDS_PER_MINUTE) % 60;
    int64_t second = secsOfDay % 60;
    // 0 and 12 both display as 12, in AM and PM respectively.
    int64_t hour12 = hour24 % 12;
    if (hour12 == 0)
      hour12 = 12;
    appendNumber(buf, hour12, 1);
    buf.push_back(u':');
    appendNumber(buf, minute, 2);
    buf.push_back(u':');
    appendNumber(buf, second, 2);
    buf.push_back(u' ');
    append(buf, hour24 < 12 ? "AM" : "PM");
  }
}

} // namespace platform_unicode
} // namespace hermes
