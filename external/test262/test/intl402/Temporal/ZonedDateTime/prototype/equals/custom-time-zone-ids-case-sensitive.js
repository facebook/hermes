// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.equals
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
const customSameCase = custom.withTimeZone(new Custom("Moon/Cheese"));
const customDifferentCase = custom.withTimeZone(new Custom("MOON/CHEESE"));

assert.sameValue(custom.equals(customSameCase), true);
assert.sameValue(custom.equals(customDifferentCase), false);
