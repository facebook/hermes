// Copyright (C) 2023 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.zoneddatetime.prototype.tolocalestring
description: Offset time zones are not supported yet by Intl
features: [Temporal]
---*/

const datetime = new Temporal.ZonedDateTime(0n, "+00:00");
assert.throws(RangeError, () => datetime.toLocaleString(), "Intl.DateTimeFormat does not yet specify what to do with offset time zones");
