// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-Intl.PluralRules.prototype.selectRange
description: >
  "selectRange" basic tests when argument cannot be converted using ToNumber
info: |
  Intl.PluralRules.prototype.selectRange ( start, end )
  (...)
  4. Let x be ? ToNumber(start).
  5. Let y be ? ToNumber(end).
locale: [en-US]
features: [Intl.NumberFormat-v3]
---*/

const pr = new Intl.PluralRules("en-US");

// Throw if arguments cannot be cast toNumber
assert.throws(TypeError, () => { pr.selectRange(Symbol(102), 201) });
assert.throws(TypeError, () => { pr.selectRange(102,Symbol(201)) });
assert.throws(TypeError, () => { pr.selectRange(23n, 100) });
assert.throws(TypeError, () => { pr.selectRange(100, 23n) });
assert.throws(TypeError, () => { pr.selectRange(23n, 23n) });
