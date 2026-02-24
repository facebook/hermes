// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
description: The duration from a date to itself is a zero duration (PT0S)
esid: sec-temporal.calendar.prototype.dateuntil
features: [Temporal]
---*/

const instance = new Temporal.Calendar("gregory");
const date = new Temporal.PlainDate(2001, 6, 3);

['year', 'month', 'week', 'day'].forEach((largestUnit) => {
  const result = instance.dateUntil(date, date, { largestUnit });
  assert.sameValue(result.toString(), 'PT0S');
});
