// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: Built-in time zones are parsed correctly out of valid strings
features: [Temporal]
---*/

const valids = [
  ["Africa/CAIRO", "Africa/Cairo"],
  ["Asia/Ulan_Bator", "Asia/Ulaanbaatar"],
  ["etc/gmt", "Etc/GMT"],
  ["1994-11-05T08:15:30-05:00[America/New_York]", "America/New_York"],
  ["1994-11-05T08:15:30+05:30[Asia/Calcutta]", "Asia/Calcutta"],
  ["1994-11-05T08:15:30+05:30[Asia/Calcutta]", "Asia/Kolkata"],
  ["1994-11-05T08:15:30+05:30[Asia/Kolkata]", "Asia/Calcutta"],
  ["1994-11-05T08:15:30+05:30[Asia/Kolkata]", "Asia/Kolkata"],
];

for (const [valid, canonical = valid] of valids) {
  const tzValid = Temporal.TimeZone.from(canonical);
  const tzCanonical = Temporal.TimeZone.from(canonical);
  assert.sameValue(tzValid.equals(tzCanonical), true);
  assert.sameValue(tzCanonical.equals(tzValid), true);
}
