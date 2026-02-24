// Copyright 2023 Google Inc.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.locale
description: >
    Checks error cases for the options argument to the Locale constructor.
info: |
    Intl.Locale( tag [, options] )

    ...
    x. Let numberingSystem be ? GetOption(options, "firstDayOfWeek", "string", < *"mon"*, *"tue"*, *"wed"*, *"thu"*, *"fri"*, *"sat"*, *"sun"*, *"0"*, *"1"*, *"2"*, *"3"*, *"4"*, *"5"*, *"6"*, *"7"*> , undefined).
    ...

features: [Intl.Locale,Intl.Locale-info]
---*/

const invalidFirstDayOfWeekOptions = [
  "",
  "m",
  "mo",
  "monday",
  true,
  false,
  null,
];
for (const firstDayOfWeek of invalidFirstDayOfWeekOptions) {
  assert.throws(RangeError, function() {
    new Intl.Locale('en', {firstDayOfWeek});
  }, `new Intl.Locale("en", {firstDayOfWeek: "${firstDayOfWeek}"}) throws RangeError`);
}
