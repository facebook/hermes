// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getnexttransition
description: >
  Test transitions at the instant boundaries.
features: [Temporal, Intl-enumeration]
---*/

const min = new Temporal.Instant(-86_40000_00000_00000_00000n);
const max = new Temporal.Instant(86_40000_00000_00000_00000n);

for (let id of Intl.supportedValuesOf("timeZone")) {
  let tz = new Temporal.TimeZone(id);

  // If there's any next transition, it should be after |min|.
  let next = tz.getNextTransition(min);
  if (next) {
    assert(next.epochNanoseconds > min.epochNanoseconds);
  }

  // There shouldn't be any next transition after |max|.
  next = tz.getNextTransition(max);
  assert.sameValue(next, null);
}
