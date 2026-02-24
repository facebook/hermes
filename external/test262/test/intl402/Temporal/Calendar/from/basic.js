// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.from
description: Support for non-ISO calendars in Calendar.from().
features: [Temporal]
---*/

function test(item, id = item) {
  const calendar = Temporal.Calendar.from(item);
  assert(calendar instanceof Temporal.Calendar, `Calendar.from(${item}) is a calendar`);
  assert.sameValue(calendar.id, id, `Calendar.from(${item}) has the correct ID`);
}
test("gregory");
test("japanese");
test("1994-11-05T08:15:30-05:00[u-ca=gregory]", "gregory");
test("1994-11-05T13:15:30Z[u-ca=japanese]", "japanese");
