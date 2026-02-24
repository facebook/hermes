// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.yearmonthfromfields
description: Reference ISO day is chosen to be the first of the calendar month
info: |
  6.d. Perform ! CreateDataPropertyOrThrow(_fields_, *"day"*, *1*<sub>𝔽</sub>).
    e. Let _result_ be ? CalendarDateToISO(_calendar_.[[Identifier]], _fields_, _options_).
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const gregory = new Temporal.Calendar("gregory");

const result1 = gregory.yearMonthFromFields({ year: 2023, monthCode: "M01", day: 13 });
TemporalHelpers.assertPlainYearMonth(
  result1,
  2023, 1, "M01",
  "reference day is 1 even if day is given",
  "ce", 2023, /* reference day = */ 1
);

const result2 = gregory.yearMonthFromFields({ year: 2021, monthCode: "M02", day: 50 }, { overflow: "constrain" });
TemporalHelpers.assertPlainYearMonth(
  result2,
  2021, 2, "M02",
  "reference day is set correctly even if day is out of range (overflow constrain)",
  "ce", 2021, /* reference day = */ 1
);

const result3 = gregory.yearMonthFromFields({ year: 2021, monthCode: "M02", day: 50 }, { overflow: "reject" });
TemporalHelpers.assertPlainYearMonth(
  result3,
  2021, 2, "M02",
  "reference day is set correctly even if day is out of range (overflow reject)",
  "ce", 2021, /* reference day = */ 1
);

const hebrew = new Temporal.Calendar("hebrew");

const result4 = hebrew.yearMonthFromFields({ year: 5782, monthCode: "M04", day: 20 });
TemporalHelpers.assertPlainYearMonth(
  result4,
  5782, 4, "M04",
  "reference day is the first of the calendar month even if day is given",
  /* era = */ undefined, /* era year = */ undefined, /* reference day = */ 5
);
const isoFields = result4.getISOFields();
assert.sameValue(isoFields.isoYear, 2021, "Tevet 5782 begins in ISO year 2021");
assert.sameValue(isoFields.isoMonth, 12, "Tevet 5782 begins in ISO month 12");

const result5 = hebrew.yearMonthFromFields({ year: 5783, monthCode: "M05L" }, { overflow: "constrain" });
TemporalHelpers.assertPlainYearMonth(
  result5,
  5783, 6, "M06",
  "month code M05L does not exist in year 5783 (overflow constrain); Hebrew calendar constrains Adar I to Adar",
  /* era = */ undefined, /* era year = */ undefined, /* reference day = */ 22
);

assert.throws(
  RangeError,
  () => hebrew.yearMonthFromFields({ year: 5783, monthCode: "M05L" }, { overflow: "reject" }),
  "month code M05L does not exist in year 5783 (overflow reject)",
);

const result6 = hebrew.yearMonthFromFields({ year: 5783, month: 13 }, { overflow: "constrain" });
TemporalHelpers.assertPlainYearMonth(
  result6,
  5783, 12, "M12",
  "month 13 does not exist in year 5783 (overflow constrain)",
  /* era = */ undefined, /* era year = */ undefined, /* reference day = */ 18
);

assert.throws(
  RangeError,
  () => hebrew.yearMonthFromFields({ year: 5783, month: 13 }, { overflow: "reject" }),
  "month 13 does not exist in year 5783 (overflow reject)",
);

const result7 = hebrew.yearMonthFromFields({ year: 5782, monthCode: "M04", day: 50 }, { overflow: "constrain" });
TemporalHelpers.assertPlainYearMonth(
  result7,
  5782, 4, "M04",
  "reference day is set correctly even if day is out of range (overflow constrain)",
  /* era = */ undefined, /* era year = */ undefined, /* reference day = */ 5
);

const result8 = hebrew.yearMonthFromFields({ year: 5782, monthCode: "M04", day: 50 }, { overflow: "reject" });
TemporalHelpers.assertPlainYearMonth(
  result8,
  5782, 4, "M04",
  "reference day is set correctly even if day is out of range (overflow reject)",
  /* era = */ undefined, /* era year = */ undefined, /* reference day = */ 5
);

const chinese = new Temporal.Calendar("chinese");

// Month codes, month indices, and the ISO reference days of the months in 2022
const months2022TestData = [
  // TODO: Sources conflict over whether M01L and M12L exist in _any_ year.
  // Clarify this, and delete if appropriate. ICU has them, but may be wrong.
  ["M01", 1, 1],
  ["M02", 2, 3],
  ["M03", 3, 1],
  ["M04", 4, 1],
  ["M05", 5, 30],
  ["M06", 6, 29],
  ["M07", 7, 29],
  ["M08", 8, 27],
  ["M09", 9, 26],
  ["M10", 10, 25],
  ["M11", 11, 24],
  ["M12", 12, 23],
];
for (const [nonLeapMonthCode, month, referenceISODay] of months2022TestData) {
  const leapMonthCode = nonLeapMonthCode + "L";
  const fields = { year: 2022, monthCode: leapMonthCode };

  const result = chinese.yearMonthFromFields(fields, { overflow: "constrain" });
  TemporalHelpers.assertPlainYearMonth(
    result,
    2022, month, nonLeapMonthCode,
    `Chinese intercalary month ${leapMonthCode} does not exist in year 2022 (overflow constrain)`,
    /* era = */ undefined, /* era year = */ undefined, referenceISODay
  );

  assert.throws(
    RangeError,
    () => chinese.yearMonthFromFields(fields, { overflow: "reject" }),
    `Chinese intercalary month ${leapMonthCode} does not exist in year 2022 (overflow reject)`
  );
}

// Years in which leap months exist according to ICU
const leapMonthsTestData = [
  ["M01L", 2148, 2, 20],
  ["M02L", 2023, 3, 22],
  ["M03L", 1993, 4, 22],
  ["M04L", 2020, 5, 23],
  ["M05L", 2009, 6, 23],
  ["M06L", 2017, 7, 23],
  ["M07L", 2006, 8, 24],
  ["M08L", 1995, 9, 25],
  ["M09L", 2014, 10, 24],
  ["M10L", 1984, 11, 23],
  ["M11L", 2033, 12, 22],
  ["M12L", 1889, 13, 21, 1890, 1],
];
for (const [monthCode, year, month, referenceISODay, isoYear = year, isoMonth = month] of leapMonthsTestData) {
  const result = chinese.yearMonthFromFields({ year, monthCode });
  TemporalHelpers.assertPlainYearMonth(
    result,
    year, month, monthCode,
    `Date of sample Chinese intercalary month ${monthCode}`,
    /* era = */ undefined, /* era year = */ undefined, referenceISODay
  );
  const isoFields = result.getISOFields();
  assert.sameValue(isoFields.isoYear, isoYear, `${year}-${monthCode} starts in ISO year ${isoYear}`);
  assert.sameValue(isoFields.isoMonth, isoMonth, `${year}-${monthCode} starts in ISO month ${isoMonth}`);
}
