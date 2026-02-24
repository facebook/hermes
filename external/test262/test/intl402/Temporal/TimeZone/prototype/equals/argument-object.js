// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: Objects with IANA IDs are compared case-insensitively with their canonical IDs
features: [Temporal]
---*/

class CustomTimeZone extends Temporal.TimeZone {
  constructor(id) {
    super("UTC");
    this._id = id;
  }
  get id() {
    return this._id;
  }
}

const classInstancesIANA = [
  new Temporal.TimeZone("Asia/Calcutta"),
  new CustomTimeZone("Asia/Calcutta"),
  new Temporal.TimeZone("Asia/Kolkata"),
  new CustomTimeZone("Asia/Kolkata"),
  new CustomTimeZone("ASIA/calcutta"),
  new CustomTimeZone("Asia/KOLKATA")
];

const plainObjectsIANA = [
  { id: "Asia/Calcutta", getPossibleInstantsFor: null, getOffsetNanosecondsFor: null },
  { id: "Asia/Kolkata", getPossibleInstantsFor: null, getOffsetNanosecondsFor: null },
  { id: "ASIA/calcutta", getPossibleInstantsFor: null, getOffsetNanosecondsFor: null },
  { id: "asia/kolkatA", getPossibleInstantsFor: null, getOffsetNanosecondsFor: null }
];

for (const object1 of classInstancesIANA) {
  for (const object2 of classInstancesIANA) {
    assert.sameValue(object1.equals(object2), true, `Receiver ${object1.id} should not equal argument ${object2.id}`);
  }
  for (const object2 of plainObjectsIANA) {
    assert.sameValue(object1.equals(object2), true, `Receiver ${object2.id} should not equal argument ${object1.id}`);
  }
}

const classInstancesIANADifferentCanonical = [
  new Temporal.TimeZone("Asia/Colombo"),
  new CustomTimeZone("Asia/Colombo"),
  new Temporal.TimeZone("ASIA/colombo"),
  new CustomTimeZone("ASIA/colombo")
];

for (const object1 of classInstancesIANADifferentCanonical) {
  for (const object2 of classInstancesIANA) {
    assert.sameValue(object1.equals(object2), false, `Receiver ${object1.id} should not equal argument ${object2.id}`);
    assert.sameValue(object2.equals(object1), false, `Receiver ${object2.id} should not equal argument ${object1.id}`);
  }
  for (const object2 of plainObjectsIANA) {
    assert.sameValue(object1.equals(object2), false, `Receiver ${object1.id} should not equal argument ${object2.id}`);
    assert.sameValue(
      object1.equals(object2.id),
      false,
      `Receiver ${object1.id} should not equal argument ${object2.id}`
    );
  }
}

const classInstancesCustomNotIANA = [new CustomTimeZone("Moon/Cheese")];
for (const object1 of classInstancesCustomNotIANA) {
  for (const object2 of classInstancesIANA) {
    assert.sameValue(object1.equals(object2), false, `Receiver ${object1.id} should not equal argument ${object2.id}`);
    assert.sameValue(object2.equals(object1), false, `Receiver ${object2.id} should not equal argument ${object1.id}`);
  }
  for (const object2 of plainObjectsIANA) {
    assert.sameValue(object1.equals(object2), false, `Receiver ${object1.id} should not equal argument ${object2.id}`);
    assert.sameValue(
      object1.equals(object2.id),
      false,
      `Receiver ${object1.id} should not equal argument ${object2.id}`
    );
  }
}
