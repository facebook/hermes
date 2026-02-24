// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar.prototype.era
description: The "era" property of Temporal.Calendar.prototype
includes: [propertyHelper.js]
features: [Temporal]
---*/

assert.sameValue(
  typeof Temporal.Calendar.prototype.era,
  "function",
  "`typeof Calendar.prototype.era` is `function`"
);

verifyProperty(Temporal.Calendar.prototype, "era", {
  writable: true,
  enumerable: false,
  configurable: true,
});
