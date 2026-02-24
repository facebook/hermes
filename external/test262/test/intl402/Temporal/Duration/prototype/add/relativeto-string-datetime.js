// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.duration.prototype.add
description: >
    Conversion of ISO date-time strings as relativeTo option to
    Temporal.ZonedDateTime or Temporal.PlainDateTime instances
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const instance = new Temporal.Duration(1, 0, 0, 1);

let relativeTo = "2019-11-01T00:00[America/Vancouver]";
const result4 = instance.add(new Temporal.Duration(0, 0, 0, 0, -24), { relativeTo });
TemporalHelpers.assertDuration(result4, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, "date-time + IANA annotation is a zoned relativeTo");

relativeTo = "2019-11-01T00:00Z[America/Vancouver]";
const result5 = instance.add(new Temporal.Duration(0, 0, 0, 0, -24), { relativeTo });
TemporalHelpers.assertDuration(result5, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, "date-time + Z + IANA annotation is a zoned relativeTo");

relativeTo = "2019-11-01T00:00-07:00[America/Vancouver]";
const result6 = instance.add(new Temporal.Duration(0, 0, 0, 0, -24), { relativeTo });
TemporalHelpers.assertDuration(result6, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, "date-time + offset + IANA annotation is a zoned relativeTo");

relativeTo = "2019-11-01T00:00+04:15[America/Vancouver]";
assert.throws(RangeError, () => instance.add(new Temporal.Duration(0, 0, 0, 0, -24), { relativeTo }), "date-time + offset + IANA annotation throws if wall time and exact time mismatch");
