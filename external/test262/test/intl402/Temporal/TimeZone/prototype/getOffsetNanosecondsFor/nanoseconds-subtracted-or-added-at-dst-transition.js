// Copyright (C) 2022 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-temporal.timezone.prototype.getoffsetnanosecondsfor
description: >
  Test offset when nanoseconds are subtracted or added from DST transition.
features: [Temporal, exponentiation]
---*/

// From <https://github.com/eggert/tz/blob/main/northamerica>:
//
// # Rule  NAME  FROM  TO    -  IN   ON       AT    SAVE  LETTER
// Rule    CA    1950  1966  -  Apr  lastSun  1:00  1:00  D
//
// # Zone  NAME             STDOFF    RULES  FORMAT  [UNTIL]
// Zone America/Los_Angeles -7:52:58  -      LMT     1883 Nov 18 12:07:02
//                          -8:00     US     P%sT    1946
//                          -8:00     CA     P%sT    1967
//                          -8:00     US     P%sT

let tz = new Temporal.TimeZone("America/Los_Angeles");
let p = Temporal.Instant.from("1965-04-25T09:00:00Z");

const nsPerHour = 60 * 60 * 1000**3;

assert.sameValue(tz.getOffsetNanosecondsFor(p),
                 -7 * nsPerHour,
                 "DST transition");

assert.sameValue(tz.getOffsetNanosecondsFor(p.add({nanoseconds: +1})),
                 -7 * nsPerHour,
                 "DST transition plus one nanosecond");

assert.sameValue(tz.getOffsetNanosecondsFor(p.add({nanoseconds: -1})),
                 -8 * nsPerHour,
                 "DST transition minus one nanosecond");
