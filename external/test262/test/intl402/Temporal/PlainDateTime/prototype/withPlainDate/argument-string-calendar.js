// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.plaindatetime.withplaindate
description: non-ISO calendars are handled correctly
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const isopdt = new Temporal.PlainDateTime(1995, 12, 7, 3, 24, 30, 123, 456, 789);
const gregorypdt = new Temporal.PlainDateTime(1995, 12, 7, 3, 24, 30, 123, 456, 789, "gregory");

const result1 = isopdt.withPlainDate("2020-11-13[u-ca=gregory]");
TemporalHelpers.assertPlainDateTime(result1, 2020, 11, "M11", 13, 3, 24, 30, 123, 456, 789,
  "result1", "ce", 2020);
assert.sameValue(result1.calendarId, "gregory", "non-ISO calendar in argument overrides ISO calendar in receiver");

const result2 = gregorypdt.withPlainDate("2020-11-13[u-ca=iso8601]");
TemporalHelpers.assertPlainDateTime(result2, 2020, 11, "M11", 13, 3, 24, 30, 123, 456, 789,
  "result2", "ce", 2020);
assert.sameValue(result2.calendarId, "gregory", "non-ISO calendar in receiver overrides ISO calendar in argument");

assert.throws(RangeError, () => gregorypdt.withPlainDate("2020-11-13[u-ca=japanese]"),
  "throws if both `this` and `other` have a non-ISO calendar");
