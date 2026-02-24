// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.era
description: An exception from TimeZone#getOffsetNanosecondsFor() is propagated.
features: [Temporal]
---*/

class TZ extends Temporal.TimeZone {
  constructor() { super("UTC") }
  getOffsetNanosecondsFor() { throw new Test262Error() }
}

const tz = new TZ();
const arg = new Temporal.ZonedDateTime(0n, tz);
const instance = new Temporal.Calendar("iso8601");

assert.throws(Test262Error, () => instance.era(arg));
