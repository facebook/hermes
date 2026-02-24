// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
description: dateUntil works as expected after a leap month in a lunisolar calendar
esid: sec-temporal.calendar.prototype.dateuntil
features: [Temporal]
---*/

const instance = new Temporal.Calendar("chinese");

// 2001 is a leap year in the Chinese calendar with a M04L leap month.
// Therefore, month: 6 is M05 in 2001 but M06 in 2000 which is not a leap year.
const one = Temporal.PlainDate.from({ year: 2000, month: 6, day: 1, calendar: 'chinese' });
const two = Temporal.PlainDate.from({ year: 2001, month: 6, day: 1, calendar: 'chinese' });

const expected = { years: 'P12M', months: 'P12M', weeks: 'P50W4D', days: 'P354D' };

Object.entries(expected).forEach(([largestUnit, expectedResult]) => {
  const actualResult = instance.dateUntil(one, two, { largestUnit });
  assert.sameValue(actualResult.toString(), expectedResult);
});
