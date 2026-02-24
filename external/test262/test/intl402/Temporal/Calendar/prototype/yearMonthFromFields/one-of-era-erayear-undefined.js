// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.yearmonthfromfields
description: Throw a TypeError if only one of era/eraYear fields is present
features: [Temporal]
---*/

const base = { year: 2000, month: 5, day: 2, era: 'ce' };
const instance = new Temporal.Calendar('gregory');
assert.throws(TypeError, () => {
  instance.yearMonthFromFields({ ...base });
});

const base2 = { year: 2000, month: 5, day: 2, eraYear: 1 };
assert.throws(TypeError, () => {
  instance.yearMonthFromFields({ ...base2 });
});
