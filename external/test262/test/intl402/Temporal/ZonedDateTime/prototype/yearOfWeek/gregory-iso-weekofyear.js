// Copyright (C) 2024 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.yearofweek
description: >
  In the ISO 8601 week calendar, calendar week number 1 of a calendar year is 
  the week including the first Thursday of that year (based on the principle 
  that a week belongs to the same calendar year as the majority of its calendar 
  days). Because of this, some calendar days of the first calendar week of a 
  calendar year may be part of the preceding date calendar year, and some 
  calendar days of the last calendar week of a calendar year may be part of
  the next calendar year.
features: [Temporal]
---*/

// 

let calendar = new Temporal.Calendar("gregory");
const date = new Temporal.ZonedDateTime(1_609_504_496_987_654_321n, "UTC", calendar);

assert.sameValue(date.yearOfWeek, 2021);

calendar = new Temporal.Calendar("iso8601");
const isodate = new Temporal.ZonedDateTime(1_609_504_496_987_654_321n, "UTC", calendar);

assert.sameValue(isodate.yearOfWeek, 2020);
