// Copyright (C) 2023 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-Intl.DurationFormat.prototype.format
description: >
  PartitionDurationFormatPattern computes on exact mathematical values.
info: |
  PartitionDurationFormatPattern ( durationFormat, duration )
  ...
  4. While done is false, repeat for each row in Table 1 in order, except the header row:
    ...
    j. If unit is "seconds", "milliseconds", or "microseconds", then
      i. If unit is "seconds", then
        1. Let nextStyle be durationFormat.[[MillisecondsStyle]].
      ...
      iv. If nextStyle is "numeric", then
        1. If unit is "seconds", then
          a. Set value to value + duration.[[Milliseconds]] / 10^3 + duration.[[Microseconds]] / 10^6 + duration.[[Nanoseconds]] / 10^9.
    ...
    l. If value is not 0 or display is not "auto", then
      ii. If style is "2-digit" or "numeric", then
        ...
        7. Let parts be ! PartitionNumberPattern(nf, value).
        ...

locale: [en-US]
includes: [testIntl.js]
features: [Intl.DurationFormat]
---*/

const durations = [
  // 10000000 + (1 / 10^9)
  // = 10000000.000000001
  {
    seconds: 10_000_000,
    nanoseconds: 1,
  },

  // 9007199254740991 + (9007199254740991 / 10^3) + (9007199254740991 / 10^6) + (9007199254740991 / 10^9)
  // = 9.016215470202185986731991 × 10^15
  {
    seconds: Number.MAX_SAFE_INTEGER,
    milliseconds: Number.MAX_SAFE_INTEGER,
    microseconds: Number.MAX_SAFE_INTEGER,
    nanoseconds: Number.MAX_SAFE_INTEGER,
  },
  {
    seconds: Number.MIN_SAFE_INTEGER,
    milliseconds: Number.MIN_SAFE_INTEGER,
    microseconds: Number.MIN_SAFE_INTEGER,
    nanoseconds: Number.MIN_SAFE_INTEGER,
  },

  // 1 + (2 / 10^3) + (3 / 10^6) + (9007199254740991 / 10^9)
  // = 9.007200256743991 × 10^6
  {
    seconds: 1,
    milliseconds: 2,
    microseconds: 3,
    nanoseconds: Number.MAX_SAFE_INTEGER,
  },

  // 9007199254740991 + (10^3 / 10^3) + (10^6 / 10^6) + (10^9 / 10^9)
  // = 9007199254740991 + 3
  // = 9007199254740994
  {
    seconds: Number.MAX_SAFE_INTEGER,
    milliseconds: 10 ** 3,
    microseconds: 10 ** 6,
    nanoseconds: 10 ** 9,
  },

  // ~1.7976931348623157e+308 / 10^9
  // = ~1.7976931348623157 × 10^299
  {
    seconds: 0,
    milliseconds: 0,
    microseconds: 0,
    nanoseconds: Number.MAX_VALUE,
  },
];

const df = new Intl.DurationFormat("en", {style: "digital"});

for (let duration of durations) {
  let expected = formatDurationFormatPattern(duration, "digital");
  assert.sameValue(
    df.format(duration),
    expected,
    `Duration is ${JSON.stringify(duration)}`
  );
}
