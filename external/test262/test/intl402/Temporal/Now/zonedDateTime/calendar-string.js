// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.now.zoneddatetime
description: String calendar argument
features: [Temporal]
---*/

const zdt = Temporal.Now.zonedDateTime("gregory");
const tz = Temporal.Now.timeZoneId();
assert(zdt instanceof Temporal.ZonedDateTime);
assert.sameValue(typeof zdt.getISOFields().calendar, "string", "calendar slot should store a string");
assert.sameValue(zdt.calendarId, "gregory");
assert.sameValue(typeof zdt.getISOFields().timeZone, "string", "time zone slot should store a string");
assert.sameValue(zdt.timeZoneId, tz);
