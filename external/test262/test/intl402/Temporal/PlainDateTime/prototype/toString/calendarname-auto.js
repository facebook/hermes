// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.plaindatetime.prototype.tostring
description: Possibly display calendar when calendarName is "auto"
features: [Temporal]
---*/

const dt = new Temporal.PlainDateTime(1976, 11, 18, 15, 23, 0, 0, 0, 0, "gregory");
const expected = "1976-11-18T15:23:00[u-ca=gregory]";

assert.sameValue(dt.toString(), expected, "shows non-ISO calendar by default (no arguments)");
assert.sameValue(dt.toString({ calendarName: "auto" }), expected, "shows only non-ISO calendar if calendarName = auto");
