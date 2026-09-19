/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

// The fixed formatter is only compiled for the backends that call it; Apple
// and Android use their platform formatters instead. See
// lib/Platform/Unicode/CMakeLists.txt and the matching select() in BUCK.
#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_JAVA && \
    HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_CF

#include "hermes/Platform/Unicode/PlatformDateFormat.h"

#include "hermes/Support/OSCompat.h"

#include "gtest/gtest.h"

#include <ctime>
#include <string>

namespace {

using namespace hermes::platform_unicode;

/// Format \p ms and return the result as a narrow string. Every character the
/// formatter emits is ASCII, so the narrowing is lossless.
std::string fmt(double ms, bool date, bool time) {
  llvh::SmallVector<char16_t, 64> buf;
  formatDateTimeFixed(ms, date, time, buf);
  std::string out;
  for (char16_t c : buf) {
    EXPECT_LT((unsigned)c, 128u) << "formatter emitted a non-ASCII character";
    out.push_back((char)c);
  }
  return out;
}

/// Pin the timezone for the duration of a test, so expectations are stable
/// regardless of the machine's configuration.
class DateFormatTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // EST with no daylight saving: a fixed -5 hour offset. localtime_r
    // caches the zone, so tzset() must follow every change -- the same
    // pattern unittests/VMRuntime/DateUtilTest.cpp uses.
    hermes::oscompat::set_env("TZ", "EST+5");
    ::tzset();
  }
  void TearDown() override {
    hermes::oscompat::unset_env("TZ");
    ::tzset();
  }
};

TEST_F(DateFormatTest, MatchesTheIcuEnUsFormat) {
  // The three shapes, against the values ICU produces today for new Date(112)
  // under TZ=EST+5. 112ms is 1969-12-31T19:00:00 local.
  EXPECT_EQ(fmt(112, true, false), "Dec 31, 1969");
  EXPECT_EQ(fmt(112, false, true), "7:00:00 PM");
  EXPECT_EQ(fmt(112, true, true), "Dec 31, 1969, 7:00:00 PM");
}

TEST_F(DateFormatTest, HandlesTheEpoch) {
  // Epoch UTC is 1969-12-31T19:00:00 in EST, the same instant as above.
  EXPECT_EQ(fmt(0, true, true), "Dec 31, 1969, 7:00:00 PM");
}

TEST_F(DateFormatTest, RoundsMillisecondsDownNotTowardZero) {
  // -1ms is before the epoch; flooring must not round it up to the epoch.
  EXPECT_EQ(fmt(-1, false, true), "6:59:59 PM");
}

TEST_F(DateFormatTest, FormatsAllTwelveMonths) {
  // 2021-01-15T12:00:00Z through 2021-12-15T12:00:00Z, chosen at midday so
  // the EST offset cannot shift the day across a month boundary.
  static const char *kExpected[] = {
      "Jan 15, 2021",
      "Feb 15, 2021",
      "Mar 15, 2021",
      "Apr 15, 2021",
      "May 15, 2021",
      "Jun 15, 2021",
      "Jul 15, 2021",
      "Aug 15, 2021",
      "Sep 15, 2021",
      "Oct 15, 2021",
      "Nov 15, 2021",
      "Dec 15, 2021"};
  static const double kMs[] = {
      1610712000000.0,
      1613390400000.0,
      1615809600000.0,
      1618488000000.0,
      1621080000000.0,
      1623758400000.0,
      1626350400000.0,
      1629028800000.0,
      1631707200000.0,
      1634299200000.0,
      1636977600000.0,
      1639569600000.0};
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(fmt(kMs[i], true, false), kExpected[i]) << "month index " << i;
}

TEST_F(DateFormatTest, UsesTwelveHourClockAtTheBoundaries) {
  // Midnight is 12 AM, not 0 AM; noon is 12 PM, not 0 PM. A naive
  // hour % 12 gets both of these wrong.
  // 2021-01-15T05:00:00Z is 00:00:00 EST.
  EXPECT_EQ(fmt(1610686800000.0, false, true), "12:00:00 AM");
  // 2021-01-15T17:00:00Z is 12:00:00 EST.
  EXPECT_EQ(fmt(1610730000000.0, false, true), "12:00:00 PM");
  // 2021-01-15T06:00:00Z is 01:00:00 EST: no leading zero on the hour.
  EXPECT_EQ(fmt(1610690400000.0, false, true), "1:00:00 AM");
  // 2021-01-15T04:59:59Z is 23:59:59 EST on the 14th.
  EXPECT_EQ(fmt(1610686799000.0, true, true), "Jan 14, 2021, 11:59:59 PM");
}

TEST_F(DateFormatTest, PadsMinutesAndSecondsButNotHourOrDay) {
  // 2021-03-05T13:04:07Z is 08:04:07 EST on the 5th.
  EXPECT_EQ(fmt(1614949447000.0, true, true), "Mar 5, 2021, 8:04:07 AM");
}

TEST_F(DateFormatTest, HandlesTheExtremesOfTheJavaScriptDateRange) {
  // +-8.64e15 ms is the range Date can represent, and is exactly 1e8 days.
  // These are far outside what time_t and localtime_r support, so they
  // exercise the equivalent-year remapping. Pinning the full string, not
  // just the year, is what actually tests that the remapping preserves the
  // month, day and time of day, not only the year.
  EXPECT_EQ(fmt(8.64e15, true, true), "Sep 12, 275760, 7:00:00 PM");
  EXPECT_EQ(fmt(-8.64e15, true, true), "Apr 19, -271821, 7:00:00 PM");
}

TEST_F(DateFormatTest, DoesNotPadYearsBelowFourDigits) {
  // CLDR's `y` field has no minimum width, so ICU prints year 499 as "499",
  // not "0499". The extreme-year test cannot catch a padding bug, because
  // six-digit years are already wider than any padding would add. Each
  // expectation below was taken from ICU (node 24, ICU 71.1, en-US,
  // dateStyle medium, TZ=EST) at the same instant.
  EXPECT_EQ(fmt(-46388764800000.0, true, false), "Dec 30, 499");
  EXPECT_EQ(fmt(-30641760000000.0, true, false), "Dec 31, 998");
  EXPECT_EQ(fmt(-30610224000000.0, true, false), "Dec 31, 999");
  EXPECT_EQ(fmt(-30578688000000.0, true, false), "Dec 31, 1000");
}

TEST_F(DateFormatTest, EmitsNothingWhenBothFlagsAreClear) {
  EXPECT_EQ(fmt(112, false, false), "");
}

TEST_F(DateFormatTest, IgnoresTheHostLocale) {
  // Setting LC_ALL alone does not change the process locale, since nothing
  // here calls setlocale(). What this guards against is a future change that
  // reads LC_ALL (or a similar variable) directly to pick a format, the way
  // PlatformUnicodeHermes.cpp already does for casing and the way ICU's
  // uloc_getDefault does for this formatter's ICU predecessor.
  std::string before = fmt(112, true, true);
  hermes::oscompat::set_env("LC_ALL", "tr_TR.UTF-8");
  std::string after = fmt(112, true, true);
  hermes::oscompat::unset_env("LC_ALL");
  EXPECT_EQ(before, after);
  EXPECT_EQ(after, "Dec 31, 1969, 7:00:00 PM");
}

} // namespace

#endif // not JAVA and not CF
