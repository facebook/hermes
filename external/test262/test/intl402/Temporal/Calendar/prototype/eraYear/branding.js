// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.erayear
description: Throw a TypeError if the receiver is invalid
features: [Symbol, Temporal]
---*/

const eraYear = Temporal.Calendar.prototype.eraYear;

assert.sameValue(typeof eraYear, "function");

assert.throws(TypeError, () => eraYear.call(undefined), "undefined");
assert.throws(TypeError, () => eraYear.call(null), "null");
assert.throws(TypeError, () => eraYear.call(true), "true");
assert.throws(TypeError, () => eraYear.call(""), "empty string");
assert.throws(TypeError, () => eraYear.call(Symbol()), "symbol");
assert.throws(TypeError, () => eraYear.call(1), "1");
assert.throws(TypeError, () => eraYear.call({}), "plain object");
assert.throws(TypeError, () => eraYear.call(Temporal.Calendar), "Temporal.Calendar");
assert.throws(TypeError, () => eraYear.call(Temporal.Calendar.prototype), "Temporal.Calendar.prototype");
