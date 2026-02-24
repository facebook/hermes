// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.plaindatetime.prototype.tostring
description: Do not show calendar (even non-ISO calendars) if calendarName = "never"
features: [Temporal]
---*/

const dt = new Temporal.PlainDateTime(1976, 11, 18, 15, 23);

assert.sameValue(
  dt.withCalendar("gregory").toString({ calendarName: "never" }),
  "1976-11-18T15:23:00",
  "omits non-ISO calendar if calendarName = never"
);
