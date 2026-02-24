// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
description: Throws if any value in the property bag is Infinity or -Infinity
esid: sec-temporal.calendar.prototype.dateadd
includes: [compareArray.js, temporalHelpers.js]
features: [Temporal]
---*/

const instance = new Temporal.Calendar("gregory");
const duration = new Temporal.Duration(1);

[Infinity, -Infinity].forEach((inf) => {
  ["constrain", "reject"].forEach((overflow) => {
    assert.throws(RangeError, () => instance.dateAdd({ era: "ad", eraYear: inf, month: 5, day: 2, calendar: instance }, duration, { overflow }), `eraYear property cannot be ${inf} (overflow ${overflow}`);

    const calls = [];
    const obj = TemporalHelpers.toPrimitiveObserver(calls, inf, "eraYear");
    assert.throws(RangeError, () => instance.dateAdd({ era: "ad", eraYear: obj, month: 5, day: 2, calendar: instance }, duration, { overflow }));
    assert.compareArray(calls, ["get eraYear.valueOf", "call eraYear.valueOf"], "it fails after fetching the primitive value");
  });
});
