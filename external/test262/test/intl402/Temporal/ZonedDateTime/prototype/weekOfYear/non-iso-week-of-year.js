// Copyright (C) 2024 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.weekofyear
description: >
  Temporal.ZonedDateTime.prototype.weekOfYear returns undefined for all 
  non-ISO calendars without a well-defined week numbering system.
features: [Temporal]
---*/

// Gregorian calendar has a well defined week-numbering system.

let calendar = new Temporal.Calendar("gregory");

// Epoch Nanoseconds for new Temporal.PlainDateTime(2024, 1, 1, 12, 34, 56, 987, 654, 321, calendar);
const date = new Temporal.ZonedDateTime(1_704_112_496_987_654_321n, "UTC", calendar);

assert.sameValue(date.weekOfYear, 1);

calendar = new Temporal.Calendar("hebrew");
const nonisodate = new Temporal.ZonedDateTime(1_704_112_496_987_654_321n, "UTC", calendar);

assert.sameValue(nonisodate.weekOfYear, undefined);
