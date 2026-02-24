// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts link names as its input.
features: [Temporal]
---*/

const testCases = [
  "Europe/Nicosia",  // Link    Asia/Nicosia    Europe/Nicosia
  "Asia/Bahrain",  // Link Asia/Qatar Asia/Bahrain
  "Antarctica/Syowa",  // Link Asia/Riyadh Antarctica/Syowa
  "Asia/Aden",  // Link Asia/Riyadh Asia/Aden      # Yemen
  "Asia/Kuwait",  // Link Asia/Riyadh Asia/Kuwait
  "Asia/Phnom_Penh",  // Link Asia/Bangkok Asia/Phnom_Penh       # Cambodia
  "Asia/Vientiane",  // Link Asia/Bangkok Asia/Vientiane        # Laos
  "Asia/Muscat",  // Link Asia/Dubai Asia/Muscat     # Oman
];

for (let id of testCases) {
  const tz = new Temporal.TimeZone(id);
  assert.sameValue(tz.id, id);
}
