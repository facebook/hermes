// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.since
description: Custom time zone IDs are compared case-sensitively
features: [Temporal]
---*/

class Custom extends Temporal.TimeZone {
  constructor(id) {
    super("UTC");
    this._id = id;
  }
  get id() {
    return this._id;
  }
}
const custom = Temporal.ZonedDateTime.from({ year: 2020, month: 1, day: 1, timeZone: new Custom("Moon/Cheese") });
const customSameCase = custom.withTimeZone(new Custom("Moon/Cheese")).with({ year: 2021 });
const customDifferentCase = custom.withTimeZone(new Custom("MOON/CHEESE")).with({ year: 2021 });

assert.sameValue(custom.until(customSameCase, { largestUnit: "year" }).toString(), "P1Y");
assert.throws(RangeError, () => custom.until(customDifferentCase, { largestUnit: "year" }));
