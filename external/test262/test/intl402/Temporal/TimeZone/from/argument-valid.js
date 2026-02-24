// Copyright (C) 2020 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: Built-in time zones are parsed correctly out of valid strings
features: [Temporal]
---*/

const valids = [
  ["Africa/Bissau"],
  ["America/Belem"],
  ["Europe/Vienna"],
  ["America/New_York"],
  ["Africa/CAIRO", "Africa/Cairo"],
  ["Asia/Ulan_Bator"],
  ["GMT"],
  ["etc/gmt", "Etc/GMT"],
  ["1994-11-05T08:15:30-05:00[America/New_York]", "America/New_York"],
  ["1994-11-05T08:15:30-05[America/New_York]", "America/New_York"],
  ["1994-11-05T08:15:30\u221205:00[America/New_York]", "America/New_York"],
  ["1994-11-05T08:15:30\u221205[America/New_York]", "America/New_York"],
];

for (const [valid, canonical = valid] of valids) {
  const result = Temporal.TimeZone.from(valid);
  assert.sameValue(Object.getPrototypeOf(result), Temporal.TimeZone.prototype);
  assert.sameValue(result.id, canonical);
  assert.sameValue(result.toString(), canonical);
}
