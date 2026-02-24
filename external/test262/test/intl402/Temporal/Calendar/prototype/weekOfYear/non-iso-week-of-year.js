// Copyright (C) 2024 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.weekofyear
description: >
  Temporal.Calendar.prototype.weekOfYear returns undefined for all 
  non-ISO calendars without a well-defined week numbering system.
features: [Temporal]
---*/

// Gregorian calendar has a well defined week-numbering system.

let calendar = new Temporal.Calendar("gregory");
const date = { month: 1, day: 1, year: 2024, calendar};

assert.sameValue(calendar.weekOfYear({...date}), 1);

calendar = new Temporal.Calendar("hebrew");
const nonisodate = { month: 1, day: 1, year: 2024, calendar};

assert.sameValue(calendar.weekOfYear({...nonisodate}), undefined);
