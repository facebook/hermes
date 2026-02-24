// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getprevioustransition
description: >
  Test previous transition when nanoseconds are subtracted resp. added to the DST transition.
features: [Temporal]
---*/

let tz = new Temporal.TimeZone("Europe/Berlin");
let p = Temporal.Instant.from("2021-03-28T01:00:00Z");

assert.sameValue(tz.getPreviousTransition(p.add({nanoseconds: -1})).toString(),
                 "2020-10-25T01:00:00Z",
                 "DST transition minus one nanosecond");

assert.sameValue(tz.getPreviousTransition(p).toString(),
                 "2020-10-25T01:00:00Z",
                 "DST transition");

assert.sameValue(tz.getPreviousTransition(p.add({nanoseconds: +1})).toString(),
                 "2021-03-28T01:00:00Z",
                 "DST transition plus one nanosecond");
