// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.duration.prototype.total
description: relativeTo string accepts an inexact UTC offset rounded to hours and minutes
features: [Temporal]
---*/

const instance = new Temporal.Duration(1, 0, 0, 0, 24);

let result;
let relativeTo;

const action = (relativeTo) => instance.total({ unit: "days", relativeTo });

relativeTo = "1970-01-01T00:00-00:45:00[-00:45]";
result = action(relativeTo);
assert.sameValue(result, 366, "ISO string offset accepted with zero seconds (string)");

relativeTo = { year: 1970, month: 1, day: 1, offset: "+00:45:00.000000000", timeZone: "+00:45" };
result = action(relativeTo);
assert.sameValue(result, 366, "ISO string offset accepted with zero seconds (property bag)");

relativeTo = "1970-01-01T00:00:00-00:45[Africa/Monrovia]";
result = action(relativeTo);
assert.sameValue(result, 366, "rounded HH:MM is accepted in string offset");

relativeTo = "1970-01-01T00:00:00-00:44:30[Africa/Monrovia]";
result = action(relativeTo);
assert.sameValue(result, 366, "unrounded HH:MM:SS is accepted in string offset");

relativeTo = "1970-01-01T00:00+00:44:30.123456789[+00:45]";
assert.throws(RangeError, () => action(relativeTo), "rounding is not accepted between ISO offset and time zone");

const timeZone = Temporal.TimeZone.from("Africa/Monrovia");
relativeTo = { year: 1970, month: 1, day: 1, offset: "+00:45", timeZone };
assert.throws(RangeError, () => action(relativeTo), "rounded HH:MM not accepted as offset in property bag");
