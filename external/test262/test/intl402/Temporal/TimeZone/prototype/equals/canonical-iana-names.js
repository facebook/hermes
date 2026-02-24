// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.from
description: Canonicalizes to evaluate time zone equality
features: [Temporal]
---*/

const neverEqual = new Temporal.TimeZone('Asia/Tokyo');
const zdt = new Temporal.ZonedDateTime(0n, 'America/Los_Angeles');
const ids = [
  ['America/Atka', 'America/Adak'],
  ['America/Knox_IN', 'America/Indiana/Knox'],
  ['Asia/Ashkhabad', 'Asia/Ashgabat'],
  ['Asia/Dacca', 'Asia/Dhaka'],
  ['Asia/Istanbul', 'Europe/Istanbul'],
  ['Asia/Macao', 'Asia/Macau'],
  ['Asia/Thimbu', 'Asia/Thimphu'],
  ['Asia/Ujung_Pandang', 'Asia/Makassar'],
  ['Asia/Ulan_Bator', 'Asia/Ulaanbaatar']
];

for (const [identifier, primaryIdentifier] of ids) {
  const tz1 = new Temporal.TimeZone(identifier);
  const tz2 = new Temporal.TimeZone(primaryIdentifier);

  // compare objects
  assert.sameValue(tz1.equals(tz2), true);
  assert.sameValue(tz2.equals(tz1), true);
  assert.sameValue(tz1.equals(neverEqual), false);

  // compare string IDs
  assert.sameValue(tz1.equals(tz2.id), true);
  assert.sameValue(tz2.equals(tz1.id), true);
  assert.sameValue(tz1.equals(neverEqual.id), false);

  // compare ZonedDateTime instances
  assert.sameValue(tz1.equals(zdt.withTimeZone(tz2)), true);
  assert.sameValue(tz2.equals(zdt.withTimeZone(tz1)), true);
  assert.sameValue(tz1.equals(zdt.withTimeZone(neverEqual)), false);

  // compare IXDTF strings
  assert.sameValue(tz1.equals(zdt.withTimeZone(tz2).toString()), true);
  assert.sameValue(tz2.equals(zdt.withTimeZone(tz1).toString()), true);
  assert.sameValue(tz1.equals(zdt.withTimeZone(neverEqual).toString()), false);
}
