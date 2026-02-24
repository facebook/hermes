// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getplaindatetimefor
description: Conversion of ISO date-time strings to Temporal.Instant instances
includes: [temporalHelpers.js]
features: [Temporal]
---*/

const instance = new Temporal.TimeZone("America/Vancouver");

let str = "1970-01-01T00:00";
assert.throws(RangeError, () => instance.getPlainDateTimeFor(str), "bare date-time string is not an instant");
str = "1970-01-01T00:00[America/Vancouver]";
assert.throws(RangeError, () => instance.getPlainDateTimeFor(str), "date-time + IANA annotation is not an instant");

str = "1970-01-01T00:00Z";
const result1 = instance.getPlainDateTimeFor(str);
TemporalHelpers.assertPlainDateTime(result1, 1969, 12, "M12", 31, 16, 0, 0, 0, 0, 0, "date-time + Z preserves exact time");

str = "1970-01-01T00:00+01:00";
const result2 = instance.getPlainDateTimeFor(str);
TemporalHelpers.assertPlainDateTime(result2, 1969, 12, "M12", 31, 15, 0, 0, 0, 0, 0, "date-time + offset preserves exact time with offset");

str = "1970-01-01T00:00Z[America/Vancouver]";
const result3 = instance.getPlainDateTimeFor(str);
TemporalHelpers.assertPlainDateTime(result3, 1969, 12, "M12", 31, 16, 0, 0, 0, 0, 0, "date-time + Z + IANA annotation ignores the IANA annotation");

str = "1970-01-01T00:00+01:00[America/Vancouver]";
const result4 = instance.getPlainDateTimeFor(str);
TemporalHelpers.assertPlainDateTime(result4, 1969, 12, "M12", 31, 15, 0, 0, 0, 0, 0, "date-time + offset + IANA annotation ignores the IANA annotation");
