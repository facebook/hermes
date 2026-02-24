// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getprevioustransition
description: >
  Test transitions at the instant boundaries.
features: [Temporal, Intl-enumeration]
---*/

const min = new Temporal.Instant(-86_40000_00000_00000_00000n);
const max = new Temporal.Instant(86_40000_00000_00000_00000n);

for (let id of Intl.supportedValuesOf("timeZone")) {
  let tz = new Temporal.TimeZone(id);

  // If there's any previous transition, it should be before |max|.
  let prev = tz.getPreviousTransition(max);
  if (prev) {
    assert(prev.epochNanoseconds < max.epochNanoseconds);
  }

  // There shouldn't be any previous transition before |min|.
  prev = tz.getPreviousTransition(min);
  assert.sameValue(prev, null);
}
