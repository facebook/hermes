// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.erayear
description: The "eraYear" property of Temporal.Calendar.prototype
includes: [propertyHelper.js]
features: [Temporal]
---*/

assert.sameValue(
  typeof Temporal.Calendar.prototype.eraYear,
  "function",
  "`typeof Calendar.prototype.eraYear` is `function`"
);

verifyProperty(Temporal.Calendar.prototype, "eraYear", {
  writable: true,
  enumerable: false,
  configurable: true,
});
