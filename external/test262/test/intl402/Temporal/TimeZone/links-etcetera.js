// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts link names as its input.
features: [Temporal]
---*/

const testCases = [
  "GMT",  // Link    Etc/GMT                         GMT
  "Etc/Universal",  // Link    Etc/UTC                         Etc/Universal
  "Etc/Zulu",  // Link    Etc/UTC                         Etc/Zulu
  "Etc/Greenwich",  // Link    Etc/GMT                         Etc/Greenwich
  "Etc/GMT-0",  // Link    Etc/GMT                         Etc/GMT-0
  "Etc/GMT+0",  // Link    Etc/GMT                         Etc/GMT+0
  "Etc/GMT0",  // Link    Etc/GMT                         Etc/GMT0
];

for (let id of testCases) {
  const tz = new Temporal.TimeZone(id);
  assert.sameValue(tz.id, id);
}
