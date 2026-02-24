// Copyright (C) 2022 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.calendar
description: Calendar names are case-insensitive
features: [Temporal]
---*/

const result = new Temporal.Calendar("jApAnEsE");
assert.sameValue(result.toString(), "japanese", "Calendar is case-insensitive");
