// Copyright (C) 2023 Richard Gibson. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-get-temporal.plainyearmonth.prototype.era
description: Validate result returned from calendar era() method
features: [Temporal]
---*/

const badResults = [
  [null, TypeError],
  [false, TypeError],
  [Infinity, TypeError],
  [-Infinity, TypeError],
  [NaN, TypeError],
  [-7, TypeError],
  [-0.1, TypeError],
  [Symbol("foo"), TypeError],
  [7n, TypeError],
  [{}, TypeError],
  [true, TypeError],
  [7.1, TypeError],
  [{valueOf() { return "7"; }}, TypeError],
];

badResults.forEach(([result, error]) => {
  const calendar = new class extends Temporal.Calendar {
    era() {
      return result;
    }
  }("iso8601");
  const instance = new Temporal.PlainYearMonth(1981, 12, calendar);
  assert.throws(error, () => instance.era, `${typeof result} ${String(result)} not converted to string`);
});

const preservedResults = [
  undefined,
  "string",
  "7",
  "7.5",
];

preservedResults.forEach(result => {
  const calendar = new class extends Temporal.Calendar {
    era() {
      return result;
    }
  }("iso8601");
  const instance = new Temporal.PlainYearMonth(1981, 12, calendar);
  assert.sameValue(instance.era, result, `${typeof result} ${String(result)} preserved`);
});
