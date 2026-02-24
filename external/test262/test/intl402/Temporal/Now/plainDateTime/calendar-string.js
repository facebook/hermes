// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.now.plaindatetime
description: String calendar argument
features: [Temporal]
---*/

const dt = Temporal.Now.plainDateTime("gregory");
assert(dt instanceof Temporal.PlainDateTime);
assert.sameValue(dt.calendarId, "gregory");
