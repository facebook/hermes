// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: >
  TimeZone constructor accepts all time zone identifiers from Intl.supportedValuesOf.
features: [Temporal, Intl-enumeration]
---*/

// Ensure all identifiers are valid and canonical.
for (let id of Intl.supportedValuesOf("timeZone")) {
  let tz = new Temporal.TimeZone(id);

  assert.sameValue(tz.id, id);
}
