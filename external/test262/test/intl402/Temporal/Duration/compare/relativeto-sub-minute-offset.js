// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.duration.compare
description: relativeTo string accepts an inexact UTC offset rounded to hours and minutes
features: [Temporal]
---*/

const duration1 = new Temporal.Duration(0, 0, 0, 31);
const duration2 = new Temporal.Duration(0, 1);

let result;
let relativeTo;

const action = (relativeTo) => Temporal.Duration.compare(duration1, duration2, { relativeTo });

relativeTo = "1970-01-01T00:00:00-00:45[Africa/Monrovia]";
result = action(relativeTo);
assert.sameValue(result, 0, "rounded HH:MM is accepted in string offset");

relativeTo = "1970-01-01T00:00:00-00:44:30[Africa/Monrovia]";
result = action(relativeTo);
assert.sameValue(result, 0, "unrounded HH:MM:SS is accepted in string offset");

const timeZone = Temporal.TimeZone.from("Africa/Monrovia");
relativeTo = { year: 1970, month: 1, day: 1, offset: "+00:45", timeZone };
assert.throws(RangeError, () => action(relativeTo), "rounded HH:MM not accepted as offset in property bag");
