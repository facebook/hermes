// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.plaindatetime.prototype.tostring
description: Show calendar when calendarName is "always"
features: [Temporal]
---*/

const dt = new Temporal.PlainDateTime(1976, 11, 18, 15, 23, 0, 0, 0, 0, "gregory");

assert.sameValue(
  dt.toString({ calendarName: "always" }),
  "1976-11-18T15:23:00[u-ca=gregory]",
  "shows non-ISO calendar if calendarName = always"
);
