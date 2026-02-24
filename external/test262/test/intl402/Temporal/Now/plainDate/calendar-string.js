// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.now.plaindate
description: String calendar argument
features: [Temporal]
---*/

const date = Temporal.Now.plainDate("gregory");
assert(date instanceof Temporal.PlainDate);
assert.sameValue(date.calendarId, "gregory");
