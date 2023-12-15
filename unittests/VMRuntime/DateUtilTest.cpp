/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "hermes/VM/JSLib/DateUtil.h"

#include <cstdlib>

using namespace hermes::vm;

namespace {
TEST(DateUtilTest, DayNumberAndTimeWithinDayTest) {
  EXPECT_EQ(0, day(0)); // Jan 1, 1970
  EXPECT_EQ(-2, day(-86400100)); // Dec 31, 1969
  EXPECT_EQ(17238, day(1489439675469)); // Mar 13, 2017

  EXPECT_EQ(0, timeWithinDay(0)); // Jan 1, 1970
  EXPECT_EQ(86399900, timeWithinDay(-86400100)); // Dec 31, 1969
  EXPECT_EQ(76475469, timeWithinDay(1489439675469)); // Mar 13, 2017
}

TEST(DateUtilTest, YearNumberTest) {
  EXPECT_EQ(365u, daysInYear(2017));
  EXPECT_EQ(366u, daysInYear(2004));
  EXPECT_EQ(365u, daysInYear(1900));
  EXPECT_EQ(366u, daysInYear(2000));
  EXPECT_EQ(365u, daysInYear(-2017));
  EXPECT_EQ(366u, daysInYear(-2004));
  EXPECT_EQ(365u, daysInYear(-1900));
  EXPECT_EQ(366u, daysInYear(-2000));

  EXPECT_EQ(17167, dayFromYear(2017));
  EXPECT_EQ(10957, dayFromYear(2000));
  EXPECT_EQ(-25567, dayFromYear(1900));

  EXPECT_EQ(1483228800000, timeFromYear(2017));
  EXPECT_EQ(946684800000, timeFromYear(2000));
  EXPECT_EQ(-2208988800000, timeFromYear(1900));

  EXPECT_EQ(2017, yearFromTime(1483228800000)); // Jan 1, 2017
  EXPECT_EQ(2016, yearFromTime(1483228790000)); // Dec 31, 2016
  EXPECT_EQ(2017, yearFromTime(1483228812000)); // Jan 1, 2017
  EXPECT_EQ(2017, yearFromTime(1489439675469)); // Mar 13, 2017
  EXPECT_EQ(2000, yearFromTime(946684803000)); // Jan 1, 2000
  EXPECT_EQ(1900, yearFromTime(-2208988800000)); // Jan 1, 1900

  EXPECT_FALSE(inLeapYear(1483228800000)); // Jan 1, 2017
  EXPECT_TRUE(inLeapYear(1483228790000)); // Dec 31, 2016
  EXPECT_FALSE(inLeapYear(1483228812000)); // Jan 1, 2017
  EXPECT_FALSE(inLeapYear(1489439675469)); // Mar 13, 2017
  EXPECT_TRUE(inLeapYear(946684803000)); // Jan 1, 2000
  EXPECT_FALSE(inLeapYear(-2208988800000)); // Jan 1, 1900
}

TEST(DateUtilTest, MonthNumberTest) {
  EXPECT_EQ(0u, monthFromTime(1483228800000)); // Jan 1, 2017
  EXPECT_EQ(11u, monthFromTime(1483228790000)); // Dec 31, 2016
  EXPECT_EQ(0u, monthFromTime(1483228812000)); // Jan 1, 2017
  EXPECT_EQ(0u, monthFromTime(946684803000)); // Jan 1, 2000
  EXPECT_EQ(0u, monthFromTime(-2208988800000)); // Jan 1, 1900
  EXPECT_EQ(1u, monthFromTime(1456732800000)); // Feb 29, 2016

  EXPECT_EQ(0u, monthFromTime(1483776000000)); // Jan 7, 2017
  EXPECT_EQ(1u, monthFromTime(1486454400000)); // Feb 7, 2017
  EXPECT_EQ(2u, monthFromTime(1488873600000)); // Mar 7, 2017
  EXPECT_EQ(3u, monthFromTime(1491548400000)); // Apr 7, 2017
  EXPECT_EQ(4u, monthFromTime(1494140400000)); // May 7, 2017
  EXPECT_EQ(5u, monthFromTime(1496818800000)); // Jun 7, 2017
  EXPECT_EQ(6u, monthFromTime(1500274800000)); // Jul 17, 2017
  EXPECT_EQ(7u, monthFromTime(1502953200000)); // Aug 17, 2017
  EXPECT_EQ(8u, monthFromTime(1505631600000)); // Sep 17, 2017
  EXPECT_EQ(9u, monthFromTime(1508223600000)); // Oct 17, 2017
  EXPECT_EQ(10u, monthFromTime(1510905600000)); // Nov 17, 2017
  EXPECT_EQ(11u, monthFromTime(1513497600000)); // Dec 17, 2017
}

TEST(DateUtilTest, DateNumberTest) {
  EXPECT_EQ(1, dateFromTime(1483228800000)); // Jan 1, 2017
  EXPECT_EQ(31, dateFromTime(1483228790000)); // Dec 31, 2016
  EXPECT_EQ(1, dateFromTime(1483228812000)); // Jan 1, 2017
  EXPECT_EQ(1, dateFromTime(946684803000)); // Jan 1, 2000
  EXPECT_EQ(1, dateFromTime(-2208988800000)); // Jan 1, 1900
  EXPECT_EQ(29, dateFromTime(1456732800000)); // Feb 29, 2016

  EXPECT_EQ(7, dateFromTime(1483776000000)); // Jan 7, 2017
  EXPECT_EQ(7, dateFromTime(1486454400000)); // Feb 7, 2017
  EXPECT_EQ(7, dateFromTime(1488873600000)); // Mar 7, 2017
  EXPECT_EQ(7, dateFromTime(1491548400000)); // Apr 7, 2017
  EXPECT_EQ(7, dateFromTime(1494140400000)); // May 7, 2017
  EXPECT_EQ(7, dateFromTime(1496818800000)); // Jun 7, 2017
  EXPECT_EQ(17, dateFromTime(1500274800000)); // Jul 17, 2017
  EXPECT_EQ(17, dateFromTime(1502953200000)); // Aug 17, 2017
  EXPECT_EQ(17, dateFromTime(1505631600000)); // Sep 17, 2017
  EXPECT_EQ(17, dateFromTime(1508223600000)); // Oct 17, 2017
  EXPECT_EQ(17, dateFromTime(1510905600000)); // Nov 17, 2017
  EXPECT_EQ(17, dateFromTime(1513497600000)); // Dec 17, 2017

  EXPECT_EQ(7, dateFromTime(1452168000000)); // Jan 7, 2016
  EXPECT_EQ(7, dateFromTime(1454846400000)); // Feb 7, 2016
  EXPECT_EQ(7, dateFromTime(1457352000000)); // Mar 7, 2016
  EXPECT_EQ(7, dateFromTime(1460030400000)); // Apr 7, 2016
  EXPECT_EQ(7, dateFromTime(1462622400000)); // May 7, 2016
  EXPECT_EQ(7, dateFromTime(1462622400000)); // Jun 7, 2016
  EXPECT_EQ(17, dateFromTime(1468756800000)); // Jul 17, 2016
  EXPECT_EQ(17, dateFromTime(1471435200000)); // Aug 17, 2016
  EXPECT_EQ(17, dateFromTime(1474113600000)); // Sep 17, 2016
  EXPECT_EQ(17, dateFromTime(1476705600000)); // Oct 17, 2016
  EXPECT_EQ(17, dateFromTime(1479384000000)); // Nov 17, 2016
  EXPECT_EQ(17, dateFromTime(1481976000000)); // Dec 17, 2016
}

TEST(DateUtilTest, WeekDayTest) {
  EXPECT_EQ(4, weekDay(0)); // Thu, Jan 1, 1970
  EXPECT_EQ(4, weekDay(100)); // Thu, Jan 1, 1970
  EXPECT_EQ(1, weekDay(1489439675469)); // Mon, Mar 13, 2017
  EXPECT_EQ(3, weekDay(23415386789000)); // Wed, Jan 3, 2712
}

TEST(DateUtilTest, LocalTZATest) {
  // On Windows, TZ env can only be set to a very limited format,
  // as documented in Microsoft Docs for _tzset. Specifically,
  // * They cannot be set to IANA TZDB zone names.
  // * Whenever DST rule is enabled (by specifying any 3 letter after the
  //   digits), Windows always uses current US DST rules. It does NOT
  //   follow historic rules for old dates or rules of other regions.

  // Ensure the local TZA is correct both in standard and daylight time zones.
  // Use the following two zones to guarantee that at least one of
  // the zones is under DST whatever day this test is ran.

  // US Pacific: DST is from Mar to Nov
#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "PST8PDT");
#else
  hermes::oscompat::set_env("TZ", "America/Los_Angeles");
#endif
  EXPECT_EQ(-2.88e+7, localTZA());

  // New Zealand: DST is from Oct to Apr
#ifdef _WINDOWS
  // This test is skipped due to Windows deficiency in TZ env variable.
#else
  hermes::oscompat::set_env("TZ", "Pacific/Auckland");
  EXPECT_EQ(4.32e+7, localTZA());
#endif

  // Disble DST entirely and make sure the TZA is the same.
  // Test both positive and negative zone.

  // Negative fixed zone
#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "PST8");
#else
  hermes::oscompat::set_env("TZ", "Etc/GMT+8");
#endif
  EXPECT_EQ(-2.88e+7, localTZA());

  // Positive fixed zone
#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "JST-9");
#else
  hermes::oscompat::set_env("TZ", "Asia/Tokyo");
#endif
  EXPECT_EQ(3.24e+7, localTZA());

  hermes::oscompat::unset_env("TZ");
}

TEST(DateUtilTest, EquivalentTimeTest) {
  // 2008-01-01, 2018-12-31
  EXPECT_EQ(1199145600, detail::equivalentTime(1199145600));
  EXPECT_EQ(1199190660, detail::equivalentTime(1199190660));
  // 2033-01-01
  EXPECT_EQ(1988150400, detail::equivalentTime(1988150400));
  // 1970-01-01
  EXPECT_EQ(0, detail::equivalentTime(0));
  // 2037-01-01
  EXPECT_EQ(2114380800, detail::equivalentTime(2114380800));
  // 1969-01-01 -> 2014-01-01, 1969-12-31 -> 2014-12-31
  EXPECT_EQ(1388534400, detail::equivalentTime(-31536000));
  EXPECT_EQ(1388579460, detail::equivalentTime(-31490940));
  // 2038-01-01 -> 2010-01-01, 2038-12-31 -> 2010-12-31
  EXPECT_EQ(1262304000, detail::equivalentTime(2145916800));
  EXPECT_EQ(1262349060, detail::equivalentTime(2145961860));
  // -123456-01-01 -> 2020-01-01, -123456-12-31 -> 2020-12-31
  EXPECT_EQ(1577836800, detail::equivalentTime(-3958062278400));
  EXPECT_EQ(1577881860, detail::equivalentTime(-3958062233340));
  // -123457-01-01 -> 2030-01-01, -123457-12-31 -> 2030-12-31
  EXPECT_EQ(1893456000, detail::equivalentTime(-3958093814400));
  EXPECT_EQ(1893501060, detail::equivalentTime(-3958093769340));
  // +123456-01-01 -> 2008-01-01, +123456-12-31 -> 2008-12-31
  EXPECT_EQ(1199145600, detail::equivalentTime(3833727840000));
  EXPECT_EQ(1199190660, detail::equivalentTime(3833727885060));
  // +123457-01-01 -> 2026-01-01, +123457-12-31 -> 2020-12-31
  EXPECT_EQ(1767225600, detail::equivalentTime(3833759462400));
  EXPECT_EQ(1767270660, detail::equivalentTime(3833759507460));
}

TEST(DateUtilTest, DaylightSavingTATest) {
  hermes::oscompat::set_env("TZ", "America/Los_Angeles");
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1489530532000)); // Mar 14, 2017
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1019514530000)); // Apr 22, 2002
  EXPECT_EQ(0, daylightSavingTA(1487111330000)); // Feb 14, 2017
  EXPECT_EQ(0, daylightSavingTA(1017700130000)); // Apr 1, 2002

  hermes::oscompat::set_env("TZ", "America/Chicago");
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1489530532000)); // Mar 14, 2017
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1019514530000)); // Apr 22, 2002
  EXPECT_EQ(0, daylightSavingTA(1487111330000)); // Feb 14, 2017
  EXPECT_EQ(0, daylightSavingTA(1017700130000)); // Apr 1, 2002

  hermes::oscompat::unset_env("TZ");
}

TEST(DateUtilTest, LocalTimeTest) {
  // On Windows, TZ env can only be set to a very limited format,
  // as documented in Microsoft Docs for _tzset. Specifically,
  // * They cannot be set to IANA TZDB zone names.
  // * Whenever DST rule is enabled (by specifying any 3 letter after the
  //   digits), Windows always uses current US DST rules. It does NOT
  //   follow historic rules for old dates or rules of other regions.

  // These test cases cover DST in effect, not in effect, and no DST.
  // It also covers positive zone and negative zone.
  // 2018-07-01T16:00:00Z
  // 2018-07-01T09:00:00-0700[America/Los_Angeles] (DST in effect)
  // 2018-07-01T12:00:00-0400[America/New_York] (DST in effect)
  // 2018-07-02T04:00:00+1200[America/New_York] (DST not in effect)
  // 2018-07-02T01:00:00+0900[Asia/Tokyo] (DST not observed)

#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "PST8PDT");
#else
  hermes::oscompat::set_env("TZ", "America/Los_Angeles");
#endif
  EXPECT_EQ(1530435600000, localTime(1530460800000));
  EXPECT_EQ(1530460800000, utcTime(1530435600000));

#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "EST5EDT");
#else
  hermes::oscompat::set_env("TZ", "America/New_York");
#endif
  EXPECT_EQ(1530446400000, localTime(1530460800000));
  EXPECT_EQ(1530460800000, utcTime(1530446400000));

#ifdef _WINDOWS
  // This test is skipped due to Windows deficiency in TZ env variable.
#else
  hermes::oscompat::set_env("TZ", "Pacific/Auckland");
  EXPECT_EQ(1530504000000, localTime(1530460800000));
  EXPECT_EQ(1530460800000, utcTime(1530504000000));
#endif

#ifdef _WINDOWS
  hermes::oscompat::set_env("TZ", "JST-9");
#else
  hermes::oscompat::set_env("TZ", "Asia/Tokyo");
#endif
  EXPECT_EQ(1530493200000, localTime(1530460800000));
  EXPECT_EQ(1530460800000, utcTime(1530493200000));

  hermes::oscompat::unset_env("TZ");
}

TEST(DateUtilTest, HoursMinutesSecondsMsTest) {
  // Uses the formulae from spec, perform sanity check.
  double t = 0;
  EXPECT_EQ(0, hourFromTime(t));
  EXPECT_EQ(0, minFromTime(t));
  EXPECT_EQ(0, secFromTime(t));
  EXPECT_EQ(0, msFromTime(t));

  // Tue, 14 Mar 2017 13:13:38.245 GMT
  t = 1489497218245;
  EXPECT_EQ(13, hourFromTime(t));
  EXPECT_EQ(13, minFromTime(t));
  EXPECT_EQ(38, secFromTime(t));
  EXPECT_EQ(245, msFromTime(t));
}

TEST(DateUtilTest, MakeTimeTest) {
  EXPECT_EQ(0, makeTime(0, 0, 0, 0));
  EXPECT_EQ(MS_PER_DAY, makeTime(24, 0, 0, 0));
  EXPECT_EQ(86399999, makeTime(23, 59, 59, 999));
  EXPECT_EQ(59126000, makeTime(16, 25, 26, 0));
  EXPECT_EQ(59126746, makeTime(16, 25, 26, 746));
}

TEST(DateUtilTest, MakeDayTest) {
  EXPECT_EQ(0, makeDay(1970, 0, 1)); // Jan 1, 1970
  EXPECT_EQ(-25567, makeDay(1900, 0, 1)); // Jan 1, 1900
  EXPECT_EQ(12403, makeDay(2003, 11, 17)); // Dec 17, 2003
  EXPECT_EQ(17240, makeDay(2017, 2, 15)); // Mar 15, 2017
}

TEST(DateUtilTest, MakeDateTest) {
  EXPECT_EQ(0, makeDate(0, 0)); // Jan 1, 1970
  EXPECT_EQ(-2208988799000, makeDate(-25567, 1000)); // Jan 1, 1900 + 1 sec
  EXPECT_EQ(1489572389000, makeDate(17240, 36389000)); // Mar 15, 2017 10:06:29
}

TEST(DateUtilTest, TimeClipTest) {
  EXPECT_TRUE(std::isnan(timeClip(std::numeric_limits<double>::infinity())));
  EXPECT_TRUE(std::isnan(timeClip(-std::numeric_limits<double>::infinity())));
  EXPECT_TRUE(std::isnan(timeClip(8.7e15)));

  EXPECT_FALSE(std::signbit(timeClip(+0.0)));
  EXPECT_EQ(0, timeClip(+0.0));
  EXPECT_FALSE(std::signbit(timeClip(-0.0)));
  EXPECT_EQ(0, timeClip(-0.0));

  EXPECT_EQ(1, timeClip(1.5));
  EXPECT_EQ(-1, timeClip(-1.5));
}
} // anonymous namespace
