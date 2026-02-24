// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone
description: IANA legacy names must be supported
features: [Temporal]
---*/

const legacyNames = [
  "Etc/GMT0",
  "GMT0",
  "GMT-0",
  "GMT+0",
  "EST5EDT", 
  "CST6CDT", 
  "MST7MDT", 
  "PST8PDT"
];

legacyNames.forEach((arg) => {
  const tz = Temporal.TimeZone.from(arg);
  assert.sameValue(tz.toString(), arg, `"${arg}" does not match "${tz.toString()}" time zone identifier`);
});
