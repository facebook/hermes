// Copyright 2023 Google Inc.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.locale
description: >
    Checks valid cases for the options argument to the Locale constructor.
info: |
    Intl.Locale.prototype.firstDayOfWeek
    3. Return loc.[[FirstDayOfWeek]].

features: [Intl.Locale,Intl.Locale-info]
---*/

const validIds = [
  ["en-u-fw-mon", 1],
  ["en-u-fw-tue", 2],
  ["en-u-fw-wed", 3],
  ["en-u-fw-thu", 4],
  ["en-u-fw-fri", 5],
  ["en-u-fw-sat", 6],
  ["en-u-fw-sun", 7],
];
for (const [id, expected] of validIds) {
  assert.sameValue(
    new Intl.Locale(id).firstDayOfWeek,
    expected,
    `new Intl.Locale(${id}).firstDayOfWeek returns "${expected}"`
  );
}
