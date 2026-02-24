// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: Basic tests for the Temporal.TimeZone constructor.
features: [Temporal]
---*/

const valid = [
  ["Europe/Vienna"],
  ["America/New_York"],
  ["Africa/CAIRO", "Africa/Cairo"],
  ["africa/cairo", "Africa/Cairo"],
  ["Asia/Ulaanbaatar"],
  ["Asia/Ulan_Bator"],
  ["UTC"],
  ["GMT"]
];
for (const [zone, id = zone] of valid) {
  const result = new Temporal.TimeZone(zone);
  assert.sameValue(typeof result, "object", `object should be created for ${zone}`);
  assert.sameValue(result.id, id, `id for ${zone} should be ${id}`);
}

const invalid = ["+00:01.1", "-01.1"];
for (const zone of invalid) {
  assert.throws(RangeError, () => new Temporal.TimeZone(zone), `should throw for ${zone}`);
}
