// Copyright (C) 2023 Justin Grant. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: Offset string time zones compare as expected
features: [Temporal]
---*/

const zdt = new Temporal.ZonedDateTime(0n, "America/Los_Angeles");
const otz1 = new Temporal.TimeZone("+05:30");
const otz2 = new Temporal.TimeZone("+0530");
const tz = new Temporal.TimeZone("Asia/Kolkata");
assert.sameValue(otz1.equals(otz2), true);
assert.sameValue(otz2.equals(otz1), true);
assert.sameValue(otz1.equals("+05:30"), true);
assert.sameValue(otz1.equals(zdt.withTimeZone(otz2)), true);
assert.sameValue(otz1.equals(zdt.withTimeZone(otz2).toString()), true);
assert.sameValue(otz1.equals(tz), false);
assert.sameValue(otz1.equals("Asia/Kolkata"), false);
assert.sameValue(otz1.equals(zdt.withTimeZone(tz)), false);
assert.sameValue(otz1.equals(zdt.withTimeZone(tz).toString()), false);
