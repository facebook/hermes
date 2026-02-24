// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.tolocalestring
description: Custom time zone names are canonicalized
features: [Temporal]
---*/

const timeZone1 = {
  id: "Asia/Kolkata",
  getPossibleInstantsFor() {},
  getOffsetNanosecondsFor() {},
};
const datetime1 = new Temporal.ZonedDateTime(0n, timeZone1);

const timeZone2 = {
  id: "Asia/Calcutta",
  getPossibleInstantsFor() {},
  getOffsetNanosecondsFor() {},
};
const datetime2 = new Temporal.ZonedDateTime(0n, timeZone2);

assert.sameValue(datetime1.toLocaleString(), datetime2.toLocaleString(), "Time zone names are canonicalized before passing to DateTimeFormat");
