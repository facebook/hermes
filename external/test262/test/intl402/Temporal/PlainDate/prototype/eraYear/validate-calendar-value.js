// Copyright (C) 2023 Richard Gibson. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-get-temporal.plaindate.prototype.erayear
description: Validate result returned from calendar eraYear() method
features: [Temporal]
---*/

const badResults = [
  [null, TypeError],
  [false, TypeError],
  [Infinity, RangeError],
  [-Infinity, RangeError],
  [NaN, RangeError],
  [-0.1, RangeError],
  ["string", TypeError],
  [Symbol("foo"), TypeError],
  [7n, TypeError],
  [{}, TypeError],
  [true, TypeError],
  [7.1, RangeError],
  ["7", TypeError],
  ["7.5", TypeError],
  [{valueOf() { return 7; }}, TypeError],
];

badResults.forEach(([result, error]) => {
  const calendar = new class extends Temporal.Calendar {
    eraYear() {
      return result;
    }
  }("iso8601");
  const instance = new Temporal.PlainDate(1981, 12, 15, calendar);
  assert.throws(error, () => instance.eraYear, `${typeof result} ${String(result)} not converted to integer`);
});

const preservedResults = [
  undefined,
  -7,
];

preservedResults.forEach(result => {
  const calendar = new class extends Temporal.Calendar {
    eraYear() {
      return result;
    }
  }("iso8601");
  const instance = new Temporal.PlainDate(1981, 12, 15, calendar);
  assert.sameValue(instance.eraYear, result, `${typeof result} ${String(result)} preserved`);
});
