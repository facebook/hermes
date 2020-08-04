// Copyright 2018 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.collator.prototype.resolvedoptions
description: Verifies the property order for the object returned by resolvedOptions().
includes: [compareArray.js]
---*/

const options = new Intl.Collator([], {
  "numeric": true,
  "caseFirst": "upper",
}).resolvedOptions();

const expected = [
  "locale",
  "usage",
  "sensitivity",
  "ignorePunctuation",
  "collation",
];

if ("numeric" in options) {
  expected.push("numeric");
}

if ("caseFirst" in options) {
  expected.push("caseFirst");
}

assert.compareArray(Object.getOwnPropertyNames(options), expected);
