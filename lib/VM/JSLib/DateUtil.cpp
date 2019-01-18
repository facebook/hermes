/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/JSLib/DateUtil.h"

#include "hermes/Platform/Unicode/PlatformUnicode.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"
#include "hermes/VM/SmallXString.h"

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <cmath>
#include <ctime>

namespace hermes {
namespace vm {

/// Perform the fmod operation and adjusts the result so that it's not negative.
/// Useful in computing dates before Jan 1 1970.
static inline double posfmod(double x, double y) {
  double result = std::fmod(x, y);
  return result < 0 ? result + y : result;
}

//===----------------------------------------------------------------------===//
// Current time

std::chrono::milliseconds::rep curTime() {
  // Use std::chrono here because we need millisecond precision, which
  // std::time() fails to provide.
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.2

double day(double t) {
  return std::floor(t / MS_PER_DAY);
}

double timeWithinDay(double t) {
  return posfmod(t, MS_PER_DAY);
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.3

/// \return true if year \p y is a leap year.
static bool isLeapYear(double y) {
  if (std::fmod(y, 4) != 0) {
    return false;
  }
  // y % 4 == 0
  if (std::fmod(y, 100) != 0) {
    return true;
  }
  // y % 100 == 0
  if (std::fmod(y, 400) != 0) {
    return false;
  }
  // y % 400 == 0
  return true;
}

uint32_t daysInYear(double y) {
  return isLeapYear(y) ? 366 : 365;
}

double dayFromYear(double y) {
  // Use the formula given in the spec for computing the day from year.
  return 365 * (y - 1970) + std::floor((y - 1969) / 4.0) -
      std::floor((y - 1901) / 100.0) + std::floor((y - 1601) / 400.0);
}

double timeFromYear(double y) {
  return MS_PER_DAY * dayFromYear(y);
}

double yearFromTime(double t) {
  if (!std::isfinite(t)) {
    // If t is infinitely in the future be done.
    return t;
  }

  // Estimate y using the average year length.
  double y = std::floor(t / (MS_PER_DAY * 365.2425)) + 1970;

  // Actual time for year y.
  double yt = timeFromYear(y);

  while (yt > t) {
    // Estimate was too high, decrement until we're correct.
    --y;
    yt = timeFromYear(y);
  }
  while (yt + daysInYear(y) * MS_PER_DAY <= t) {
    // t is more than a year away from the start of y.
    // Increment y until we're correct.
    ++y;
    yt = timeFromYear(y);
  }

  assert(
      timeFromYear(y) <= t && timeFromYear(y + 1) > t &&
      "yearFromTime incorrect");
  return y;
}

bool inLeapYear(double t) {
  return daysInYear(yearFromTime(t)) == 366;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.4

uint32_t monthFromTime(double t) {
  double dayWithinYear = day(t) - dayFromYear(yearFromTime(t));
  constexpr int8_t kDaysInMonthNonLeap[11] = {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};
  double curDay = 0.0;
  for (uint32_t i = 0; i < 11; ++i) {
    curDay += (i == 1 && inLeapYear(t)) ? kDaysInMonthNonLeap[i] + 1
                                        : kDaysInMonthNonLeap[i];
    if (dayWithinYear < curDay)
      return i;
  }
  // Must be December.
  return 11;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.5

/// Gives the offset of the first day in month m.
/// \param leap indicates if \p m falls in a leap year.
static uint32_t dayFromMonth(uint32_t m, bool leap) {
  static const uint16_t standardTable[]{
      0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  static const uint16_t leapYearTable[]{
      0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};
  assert(m < 12 && "invalid month supplied to dayFromMonth");
  return leap ? leapYearTable[m] : standardTable[m];
}

double dateFromTime(double t) {
  double dayWithinYear = day(t) - dayFromYear(yearFromTime(t));
  bool leap = inLeapYear(t);
  return dayWithinYear - dayFromMonth(monthFromTime(t), leap) + 1;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.6

double weekDay(double t) {
  return posfmod((day(t) + 4), 7);
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.7

double localTZA() {
  ::tzset();

  // Get the current time in seconds (might have DST adjustment included).
  time_t currentWithDST = std::time(nullptr);
  if (currentWithDST == static_cast<time_t>(-1)) {
    return 0;
  }

  // Deconstruct the time into localTime.
  std::tm *local = std::localtime(&currentWithDST);
  if (!local) {
    llvm_unreachable("localtime failed in localTZA()");
  }

#ifdef _WINDOWS
  long gmtoff = -_timezone;
#else
  long gmtoff = local->tm_gmtoff;
#endif

  // Use the gmtoff field and subtract an hour if currently in DST.
  return (gmtoff * MS_PER_SECOND) - (local->tm_isdst ? MS_PER_HOUR : 0);
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.8

double daylightSavingTA(double t) {
  if (!std::isfinite(t)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  ::tzset();

  // Convert t to seconds and get the actual time needed.
  const double seconds = t / MS_PER_SECOND;
  // If the number of seconds is higher or lower than a unix timestamp can
  // support, clamp it. This is not correct in all cases, but returning NaN (for
  // Invalid Date) breaks date construction entirely. Clamping only results in
  // small errors in daylight savings time. This is only a problem in systems
  // with a 32-bit time_t, like some Android systems.
  time_t local = 0;
  if (seconds > std::numeric_limits<time_t>::max()) {
    local = std::numeric_limits<time_t>::max();
  } else if (seconds < std::numeric_limits<time_t>::min()) {
    local = std::numeric_limits<time_t>::min();
  } else {
    // This will truncate any fractional seconds, which is ok for daylight
    // savings time calculations.
    // The integral part of the float is guaranteed to be in bounds by the above
    // checks.
    local = seconds;
  }

  std::tm *brokenTime = std::localtime(&local);
  if (!brokenTime) {
    // Local time is invalid.
    return std::numeric_limits<double>::quiet_NaN();
  }
  return brokenTime->tm_isdst ? MS_PER_HOUR : 0;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.9

/// Conversion from UTC to local time.
double localTime(double t) {
  return t + localTZA() + daylightSavingTA(t);
}

/// Conversion from local time to UTC.
double utcTime(double t) {
  double ltza = localTZA();
  return t - ltza - daylightSavingTA(t - ltza);
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.10

double hourFromTime(double t) {
  return posfmod(std::floor(t / MS_PER_HOUR), HOURS_PER_DAY);
}

double minFromTime(double t) {
  return posfmod(std::floor(t / MS_PER_MINUTE), MINUTES_PER_HOUR);
}

double secFromTime(double t) {
  return posfmod(std::floor(t / MS_PER_SECOND), SECONDS_PER_MINUTE);
}

double msFromTime(double t) {
  return posfmod(t, MS_PER_SECOND);
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.11

double makeTime(double hour, double min, double sec, double ms) {
  if (!std::isfinite(hour) || !std::isfinite(min) || !std::isfinite(sec) ||
      !std::isfinite(ms)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  double h = oscompat::trunc(hour);
  double m = oscompat::trunc(min);
  double s = oscompat::trunc(sec);
  double milli = trunc(ms);
  return h * MS_PER_HOUR + m * MS_PER_MINUTE + s * MS_PER_SECOND + milli;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.12

double makeDay(double year, double month, double date) {
  if (!std::isfinite(year) || !std::isfinite(month) || !std::isfinite(date)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  double y = oscompat::trunc(year);
  double m = oscompat::trunc(month);
  double dt = oscompat::trunc(date);

  // Actual year and month, accounting for the month being greater than 11.
  // Need to do this because it changes the leap year calculations.
  double ym = y + std::floor(m / 12);
  double mn = posfmod(m, 12);

  bool leap = isLeapYear(ym);

  // Day of the first day of the year ym.
  double yd = std::floor(timeFromYear(ym) / MS_PER_DAY);
  // Day of the first day of the month mn.
  double md = dayFromMonth(mn, leap);

  // Final date is the first day of the year, offset by the first day of the
  // month, with dt - 1 to account for the day within the month.
  return yd + md + dt - 1;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.13

double makeDate(double day, double t) {
  if (!std::isfinite(day) || !std::isfinite(t)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return day * MS_PER_DAY + t;
}

//===----------------------------------------------------------------------===//
// ES5.1 15.9.1.14

double timeClip(double t) {
  if (!std::isfinite(t) || std::abs(t) > 8.64e15) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Truncate and make -0 into +0.
  return oscompat::trunc(t) + 0;
}

//===----------------------------------------------------------------------===//
// toString Functions

void dateToString(double t, double, llvm::SmallVectorImpl<char> &buf) {
  llvm::raw_svector_ostream os{buf};

  /// Make these ints here because we're printing and we have bounds on
  /// their values. Makes printing very easy.
  int32_t y = yearFromTime(t);
  int32_t m = monthFromTime(t) + 1; // monthFromTime(t) is 0-indexed.
  int32_t d = dateFromTime(t);

  if (y < 0 || y > 9999) {
    // Handle extended years.
    os << llvm::format("%+07d-%02d-%02d", y, m, d);
  } else {
    os << llvm::format("%04d-%02d-%02d", y, m, d);
  }
}

void timeToString(double t, double tza, llvm::SmallVectorImpl<char> &buf) {
  llvm::raw_svector_ostream os{buf};

  /// Make all of these ints here because we're printing and we have bounds on
  /// their values. Makes printing very easy.
  int32_t h = hourFromTime(t);
  int32_t min = minFromTime(t);
  int32_t s = secFromTime(t);
  int32_t ms = msFromTime(t);

  if (tza == 0) {
    // Zulu time, output Z as the time zone.
    os << llvm::format("%02d:%02d:%02d.%03dZ", h, min, s, ms);
  } else {
    // Calculate the +HH:mm expression for the time zone adjustment.
    // First account for the sign, then perform calculations on positive TZA.
    char sign = tza >= 0 ? '+' : '-';
    double tzaPos = std::abs(tza);
    int32_t tzh = hourFromTime(tzaPos);
    int32_t tzm = minFromTime(tzaPos);
    os << llvm::format(
        "%02d:%02d:%02d.%03d%c%02d:%02d", h, min, s, ms, sign, tzh, tzm);
  }
}

static void datetimeToString(
    double t,
    double tza,
    llvm::SmallVectorImpl<char> &buf,
    char separator) {
  dateToString(t, tza, buf);
  buf.push_back(separator);
  timeToString(t, tza, buf);
}

void datetimeToISOString(
    double t,
    double tza,
    llvm::SmallVectorImpl<char> &buf) {
  return datetimeToString(t, tza, buf, 'T');
}

void datetimeToUTCString(
    double t,
    double tza,
    llvm::SmallVectorImpl<char> &buf) {
  return datetimeToString(t, tza, buf, ' ');
}

void datetimeToLocaleString(double t, llvm::SmallVectorImpl<char16_t> &buf) {
  return platform_unicode::dateFormat(t, true, true, buf);
}

void dateToLocaleString(double t, llvm::SmallVectorImpl<char16_t> &buf) {
  return platform_unicode::dateFormat(t, true, false, buf);
}

void timeToLocaleString(double t, llvm::SmallVectorImpl<char16_t> &buf) {
  return platform_unicode::dateFormat(t, false, true, buf);
}

//===----------------------------------------------------------------------===//
// Date parsing

/// \return true if c represents a digit between 0 and 9.
static inline bool isDigit(char16_t c) {
  return u'0' <= c && c <= u'9';
}

/// Read a number from the iterator at \p it into \p x.
/// Can read integers that consist entirely of digits.
/// \param[in][out] is modified to the new start point of the scan if
/// successful.
/// \param end the end of the string.
/// \param[out] x modified to contain the scanned integer.
/// \return true if successful, false if failed.
template <class InputIter>
static bool scanInt(InputIter &it, const InputIter end, int32_t &x) {
  llvm::SmallString<16> str{};
  if (it == end) {
    return false;
  }
  for (; it != end && isDigit(*it); ++it) {
    str += static_cast<char>(*it);
  }
  llvm::StringRef ref{str};
  // getAsInteger returns false to signify success.
  return !ref.getAsInteger(10, x);
}

double parseDate(StringView u16str) {
  const double nan = std::numeric_limits<double>::quiet_NaN();

  auto it = u16str.begin();
  auto end = u16str.end();

  // Used to indicate the negation multiplier on an integer.
  // 1 for positive, -1 for negative.
  double sign;

  // Initialize these fields to their defaults.
  int32_t y, m{1}, d{1}, h{0}, min{0}, s{0}, ms{0}, tzh{0}, tzm{0};

  auto consume = [&](char16_t ch) {
    if (it != end && *it == ch) {
      ++it;
      return true;
    }
    return false;
  };

  // Must read the year.
  sign = 1;
  if (consume(u'+')) {
    sign = 1;
  } else if (consume(u'-')) {
    sign = -1;
  }
  if (!scanInt(it, end, y)) {
    return nan;
  }
  y *= sign;
  if (consume(u'-')) {
    // Try to read the month.
    if (!scanInt(it, end, m)) {
      return nan;
    }
    if (consume(u'-')) {
      // Try to read the date.
      if (!scanInt(it, end, d)) {
        return nan;
      }
    }
  }

  // See if there's a time.
  if (consume(u'T') || consume(u' ')) {
    // Hours and minutes must exist.
    if (!scanInt(it, end, h)) {
      return nan;
    }
    if (!consume(u':')) {
      return nan;
    }
    if (!scanInt(it, end, min)) {
      return nan;
    }
    if (consume(u':')) {
      // Try to read seconds.
      if (!scanInt(it, end, s)) {
        return nan;
      }
      if (consume(u'.')) {
        // Try to read milliseconds.
        if (!scanInt(it, end, ms)) {
          return nan;
        }
      }
    }

    // Try to parse a timezone.
    if (consume(u'Z')) {
      tzh = 0;
      tzm = 0;
    } else if (it != end) {
      // Try to parse the fully specified timezone: [+/-]HH:mm.
      if (consume(u'+')) {
        sign = 1;
      } else if (consume(u'-')) {
        sign = -1;
      } else {
        // Need a + or a -.
        return nan;
      }
      if (!scanInt(it, end, tzh)) {
        return nan;
      }
      tzh *= sign;
      if (!consume(u':')) {
        return nan;
      }
      if (!scanInt(it, end, tzm)) {
        return nan;
      }
    }
  }

  if (it != end) {
    // Should be done parsing.
    return nan;
  }

  // Account for the fact that m was 1-indexed and the timezone offset.
  return makeDate(makeDay(y, m - 1, d), makeTime(h - tzh, min - tzm, s, ms));
}

} // namespace vm
} // namespace hermes
