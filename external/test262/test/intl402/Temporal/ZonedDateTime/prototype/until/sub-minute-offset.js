// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.until
description: Fuzzy matching behaviour for UTC offset in ISO 8601 string with named time zones
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const timeZone = new Temporal.TimeZone("Africa/Monrovia");
const instance = new Temporal.ZonedDateTime(0n, timeZone);

let result = instance.until("1970-01-01T00:44:30-00:44:30[Africa/Monrovia]");
TemporalHelpers.assertDuration(result, 0, 0, 0, 0, 1, 29, 0, 0, 0, 0, "UTC offset rounded to minutes is accepted");

result = instance.until("1970-01-01T00:44:30-00:44:30[Africa/Monrovia]");
TemporalHelpers.assertDuration(result, 0, 0, 0, 0, 1, 29, 0, 0, 0, 0, "Unrounded sub-minute UTC offset also accepted");

assert.throws(
  RangeError,
  () => instance.until("1970-01-01T00:44:30+00:44:30[+00:45"),
  "minute rounding not supported for offset time zones"
);

const properties = {
  offset: "-00:45",
  year: 1970,
  month: 1,
  day: 1,
  minute: 44,
  second: 30,
  timeZone
};
assert.throws(RangeError, () => instance.until(properties), "no fuzzy matching is done on offset in property bag");
