// Copyright 2023 Google Inc.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.locale
description: >
    Checks valid cases for the options argument to the Locale constructor.
info: |
    Intl.Locale( tag [, options] )

    ...
    x. Let numberingSystem be ? GetOption(options, "firstDayOfWeek", "string", < *"mon"*, *"tue"*, *"wed"*, *"thu"*, *"fri"*, *"sat"*, *"sun"*, *"0"*, *"1"*, *"2"*, *"3"*, *"4"*, *"5"*, *"6"*, *"7"*> , undefined).
    x. Let firstDay be *undefined*.
    x. If fw is not *undefined*, then
       x. Set firstDay to !WeekdayToString(fw).
       x. Set opt.[[fw]] to firstDay.
    ...
    x. Let r be ! ApplyUnicodeExtensionToTag(tag, opt, relevantExtensionKeys).
    ...
    x. Let firstDay be *undefined*.
    x. If r.[[fw]] is not *undefined*, then
       x. Set firstDay to ! WeekdayToNumber(r.[[fw]]).
       x. Set locale.[[FirstDayOfWeek]] to firstDay.
    ...

features: [Intl.Locale,Intl.Locale-info]
---*/

const validFirstDayOfWeekOptions = [
  ["mon", "en-u-fw-mon"],
  ["tue", "en-u-fw-tue"],
  ["wed", "en-u-fw-wed"],
  ["thu", "en-u-fw-thu"],
  ["fri", "en-u-fw-fri"],
  ["sat", "en-u-fw-sat"],
  ["sun", "en-u-fw-sun"],
  ["1", "en-u-fw-mon"],
  ["2", "en-u-fw-tue"],
  ["3", "en-u-fw-wed"],
  ["4", "en-u-fw-thu"],
  ["5", "en-u-fw-fri"],
  ["6", "en-u-fw-sat"],
  ["7", "en-u-fw-sun"],
  ["0", "en-u-fw-sun"],
  [1, "en-u-fw-mon"],
  [2, "en-u-fw-tue"],
  [3, "en-u-fw-wed"],
  [4, "en-u-fw-thu"],
  [5, "en-u-fw-fri"],
  [6, "en-u-fw-sat"],
  [7, "en-u-fw-sun"],
  [0, "en-u-fw-sun"],
];
for (const [firstDayOfWeek, expected] of validFirstDayOfWeekOptions) {
  assert.sameValue(
    new Intl.Locale('en', { firstDayOfWeek }).toString(),
    expected,
    `new Intl.Locale("en", { firstDayOfWeek: ${firstDayOfWeek} }).toString() returns "${expected}"`
  );
  assert.sameValue(
    new Intl.Locale('en-u-fw-WED', { firstDayOfWeek }).toString(),
    expected,
    `new Intl.Locale("en-u-fw-WED", { firstDayOfWeek: ${firstDayOfWeek} }).toString() returns "${expected}"`
  );
}
