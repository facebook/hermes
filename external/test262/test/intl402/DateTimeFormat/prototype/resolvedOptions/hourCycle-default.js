// Copyright 2019 Google Inc., 2023 Igalia S.L. All rights reserved.
// This code is governed by the license found in the LICENSE file.

/*---
esid: sec-Intl.DateTimeFormat.prototype.resolvedOptions
description: >
  Intl.DateTimeFormat.prototype.resolvedOptions properly
  reflect hourCycle settings.
info: |
  11.3.7 Intl.DateTimeFormat.prototype.resolvedOptions()

  11.1.2 CreateDateTimeFormat ( dateTimeFormat, locales, options, required, defaults )
   23. Let dataLocaleData be localeData.[[<dataLocale>]].
   24. If hour12 is true, then
       a. Let hc be dataLocaleData.[[hourCycle12]].
   25. Else if hour12 is false, then
       a. Let hc be dataLocaleData.[[hourCycle24]].
   26. Else,
        a. Assert: hour12 is undefined.
        b. Let hc be r.[[hc]].
        c. If hc is null, set hc to dataLocaleData.[[hourCycle]].
  27. Set dateTimeFormat.[[HourCycle]] to hc.

locale: [en, fr, it, ja, zh, ko, ar, hi, en-u-hc-h24]
---*/

let locales = ["en", "fr", "it", "ja", "ja-u-hc-h11", "zh", "ko", "ar", "hi", "en-u-hc-h24"];

locales.forEach(function(locale) {
  let hcDefault = new Intl.DateTimeFormat(locale, { hour: "numeric" }).resolvedOptions().hourCycle;
  if (hcDefault === "h11" || hcDefault === "h12") {
    assert.sameValue(new Intl.DateTimeFormat(locale, { hour: "numeric", hour12: true }).resolvedOptions().hourCycle, hcDefault);

    // no locale has "h24" as a default. see https://github.com/tc39/ecma402/pull/758#issue-1622377292
    assert.sameValue(new Intl.DateTimeFormat(locale, { hour: "numeric", hour12: false }).resolvedOptions().hourCycle, "h23");
  }

  // however, "h24" can be set via locale extension.
  if (hcDefault === "h23" || hcDefault === "h24") {
    assert.sameValue(new Intl.DateTimeFormat(locale, { hour: "numeric", hour12: false }).resolvedOptions().hourCycle, hcDefault);
  }

  let hcHour12 = new Intl.DateTimeFormat(locale, { hour: "numeric", hour12: true }).resolvedOptions().hourCycle;
  assert(hcHour12 === "h11" || hcHour12 === "h12", "Expected `hourCycle`: " + hcHour12 + " to be in [\"h11\", \"h12\"]");
});
