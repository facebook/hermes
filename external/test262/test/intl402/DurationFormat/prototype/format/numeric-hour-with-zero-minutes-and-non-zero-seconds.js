// Copyright (C) 2023 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-Intl.DurationFormat.prototype.format
description: >
  The correct separator is used for numeric hours with zero minutes and non-zero seconds.
locale: [en-US]
features: [Intl.DurationFormat]
---*/

const df = new Intl.DurationFormat("en", {
  // hours must be numeric, so that a time separator is used for the following units.
  hours: "numeric",
});

const lf = new Intl.ListFormat("en", {
  type: "unit",
  style: "short",
});

const duration = {
  hours: 1,

  // Minutes is omitted from the output when its value is zero.
  minutes: 0,

  // Either seconds or sub-seconds must be non-zero.
  seconds: 3,
};

const expected = lf.format([
  new Intl.NumberFormat("en", {minimumIntegerDigits: 1}).format(duration.hours),
  new Intl.NumberFormat("en", {minimumIntegerDigits: 2}).format(duration.seconds),
]);

assert.sameValue(
  df.format(duration),
  expected,
  `No time separator is used when minutes is zero`
);
