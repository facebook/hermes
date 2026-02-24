// Copyright (C) 2020 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: An object is returned unchanged
features: [Temporal]
---*/

class CustomTimeZone extends Temporal.TimeZone {}

const objects = [
  new Temporal.TimeZone("Europe/Madrid"),
  new CustomTimeZone("Africa/Accra"),
];

const thisValues = [
  Temporal.TimeZone,
  CustomTimeZone,
  {},
  null,
  undefined,
  7,
];

for (const thisValue of thisValues) {
  for (const object of objects) {
    const result = Temporal.TimeZone.from.call(thisValue, object);
    assert.sameValue(result, object);
  }

  const zdt = new Temporal.ZonedDateTime(0n, "Africa/Cairo");
  const fromZdt = Temporal.TimeZone.from.call(thisValue, zdt);
  assert.notSameValue(fromZdt, zdt.getTimeZone(), "from() creates a new object for a string slot value");
  assert.sameValue(fromZdt.id, "Africa/Cairo");
}
