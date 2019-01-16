#include "TestHelpers.h"

#include "hermes/VM/JSLib/DateUtil.h"

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
  // Ensure the local TZA is correct both in standard and daylight time zones.
  setenv("TZ", "America/Los_Angeles", 1);
  EXPECT_EQ(-2.88e+7, localTZA());

  // Disble DST entirely and make sure the TZA is the same.
  setenv("TZ", "PST+8", 1);
  EXPECT_EQ(-2.88e+7, localTZA());

  unsetenv("TZ");
}

TEST(DateUtilTest, DaylightSavingTATest) {
  // Note that the TZ env variable needs the "PDT" part to allow for the option
  // of daylight time.
  // Try this with DST modes and make sure they work.
  setenv("TZ", "America/Los_Angeles", 1);
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1489530532000)); // Mar 14, 2017
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1019514530000)); // Apr 22, 2002
  EXPECT_EQ(0, daylightSavingTA(1487111330000)); // Feb 14, 2017
  EXPECT_EQ(0, daylightSavingTA(1017700130000)); // Apr 1, 2002

  setenv("TZ", "America/Chicago", 1);
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1489530532000)); // Mar 14, 2017
  EXPECT_EQ(MS_PER_HOUR, daylightSavingTA(1019514530000)); // Apr 22, 2002
  EXPECT_EQ(0, daylightSavingTA(1487111330000)); // Feb 14, 2017
  EXPECT_EQ(0, daylightSavingTA(1017700130000)); // Apr 1, 2002

  unsetenv("TZ");
}

TEST(DateUtilTest, LocalTimeTest) {
  // UTC: Mon, 22 Apr 2002 13:37:00 GMT
  // PDT: Mon, 22 Apr 2002 06:37:00 PDT (-0700)
  // CDT: Mon, 22 Apr 2002 08:37:00 PDT (-0700)
  // PST: Mon, 22 Apr 2002 05:37:00 PST (-0800)
  setenv("TZ", "America/Los_Angeles", 1);
  EXPECT_EQ(1019457420000, localTime(1019482620000));
  EXPECT_EQ(1019482620000, utcTime(1019457420000));

  setenv("TZ", "America/Chicago", 1);
  EXPECT_EQ(1019464620000, localTime(1019482620000));
  EXPECT_EQ(1019482620000, utcTime(1019464620000));

  // Disable DST entirely and test the conversions.
  setenv("TZ", "PST+8", 1);
  EXPECT_EQ(1019453820000, localTime(1019482620000));
  EXPECT_EQ(1019482620000, utcTime(1019453820000));

  unsetenv("TZ");
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
