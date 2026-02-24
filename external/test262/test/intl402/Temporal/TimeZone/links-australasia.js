// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts link names as its input.
features: [Temporal]
---*/

const testCases = [
  "Pacific/Saipan",  // Link Pacific/Guam Pacific/Saipan # N Mariana Is
  "Antarctica/McMurdo",  // Link Pacific/Auckland Antarctica/McMurdo
  "Antarctica/DumontDUrville",  // Link Pacific/Port_Moresby Antarctica/DumontDUrville
  "Pacific/Midway",  // Link Pacific/Pago_Pago Pacific/Midway # in US minor outlying islands
];

for (let id of testCases) {
  const tz = new Temporal.TimeZone(id);
  assert.sameValue(tz.id, id);
}
