// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.tolocalestring
description: >
  Custom time zones with unofficial names are not supported for locale formatting
features: [Temporal]
---*/

const timeZone = {
  id: "Etc/Custom_Zone",
  getPossibleInstantsFor() {},
  getOffsetNanosecondsFor() {},
};
const datetime = new Temporal.ZonedDateTime(0n, timeZone);
assert.throws(RangeError, () => datetime.toLocaleString(), "Custom time zones with non-IANA identifiers not supported in Intl");
