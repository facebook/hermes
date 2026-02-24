// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.monthdayfromfields
description: Deterministic choosing of the reference year
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const gregory = new Temporal.Calendar("gregory");

const result1 = gregory.monthDayFromFields({ year: 2021, monthCode: "M02", day: 29 });
TemporalHelpers.assertPlainMonthDay(
  result1, "M02", 29,
  "year is ignored and reference year should be 1972 if monthCode is given",
  1972
);

const result2 = gregory.monthDayFromFields({ year: 2021, month: 2, day: 29 }, { overflow: "constrain" });
TemporalHelpers.assertPlainMonthDay(
  result2, "M02", 28,
  "if monthCode is not given, year is used to determine if calendar date exists, but reference year should still be 1972",
  1972
);

assert.throws(
  RangeError,
  () => gregory.monthDayFromFields({ year: 2021, month: 2, day: 29 }, { overflow: "reject" }),
  "RangeError thrown if calendar date does not exist in given year and overflow is reject"
);

const hebrew = new Temporal.Calendar("hebrew");

const result3 = hebrew.monthDayFromFields({ monthCode: "M01", day: 1 });
TemporalHelpers.assertPlainMonthDay(
  result3, "M01", 1,
  "reference year should be 1972 if date exists in 1972",
  1972
);

const result4 = hebrew.monthDayFromFields({ monthCode: "M05L", day: 1 });
TemporalHelpers.assertPlainMonthDay(
  result4, "M05L", 1,
  "reference year should be the latest ISO year before 1972 if date does not exist in 1972",
  1970
);

const result5 = hebrew.monthDayFromFields({ year: 5781, monthCode: "M02", day: 30 });
TemporalHelpers.assertPlainMonthDay(
  result5, "M02", 30,
  "year is ignored if monthCode is given (Cheshvan 5781 has 29 days)",
  1971
);

const result6 = hebrew.monthDayFromFields({ year: 5781, month: 2, day: 30 }, { overflow: "constrain" });
TemporalHelpers.assertPlainMonthDay(
  result6, "M02", 29,
  "if monthCode is not given, year is used to determine if calendar date exists, but reference year still correct",
  1972
);

assert.throws(
  RangeError,
  () => hebrew.monthDayFromFields({ year: 5781, month: 2, day: 30 }, { overflow: "reject" }),
  "RangeError thrown if calendar date does not exist in given year and overflow is reject"
);

const result7 = hebrew.monthDayFromFields({ monthCode: "M04", day: 26 });
TemporalHelpers.assertPlainMonthDay(
  result7, "M04", 26,
  "reference date should be the later one, if two options exist in ISO year 1972",
  1972
);
assert.sameValue(result7.getISOFields().isoMonth, 12, "reference date should be 1972-12-31");
assert.sameValue(result7.getISOFields().isoDay, 31, "reference date should be 1972-12-31");
