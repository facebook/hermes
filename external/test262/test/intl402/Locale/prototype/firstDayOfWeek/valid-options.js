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

const validFirstDayOfWeekOptions = [
  ["mon", 1],
  ["tue", 2],
  ["wed", 3],
  ["thu", 4],
  ["fri", 5],
  ["sat", 6],
  ["sun", 7],
  ["1", 1],
  ["2", 2],
  ["3", 3],
  ["4", 4],
  ["5", 5],
  ["6", 6],
  ["7", 7],
  ["0", 7],
  [1, 1],
  [2, 2],
  [3, 3],
  [4, 4],
  [5, 5],
  [6, 6],
  [7, 7],
  [0, 7],
];
for (const [firstDayOfWeek, expected] of validFirstDayOfWeekOptions) {
  assert.sameValue(
    new Intl.Locale('en', { firstDayOfWeek }).firstDayOfWeek,
    expected,
    `new Intl.Locale("en", { firstDayOfWeek: ${firstDayOfWeek} }).firstDayOfWeek returns "${expected}"`
  );
  assert.sameValue(
    new Intl.Locale('en-u-fw-WED', { firstDayOfWeek }).firstDayOfWeek,
    expected,
    `new Intl.Locale("en-u-fw-WED", { firstDayOfWeek: ${firstDayOfWeek} }).firstDayOfWeek returns "${expected}"`
  );
}
