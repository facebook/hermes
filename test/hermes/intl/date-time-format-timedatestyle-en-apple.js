/**
 * Copyright 2019 Igalia, S.L. All rights reserved.
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Based on test262/test/intl402/DateTimeFormat/prototype/format/timedatestyle-en.js
 *
 * This original code this file is based on is governed by the
 * BSD license found in the external/test/262/LICENSE file.
 *
 * This additions and modifications to the original source are licensed
 * under the MIT license found in the LICENSE file in the root directory
 * of this source tree.
 */

// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

// Tolerate implementation variance by expecting consistency without being prescriptive.
// TODO: can we change tests to be less reliant on CLDR formats while still testing that
// Temporal and Intl are behaving as expected?
const usDayPeriodSpace =
  new Intl.DateTimeFormat("en-US", { timeStyle: "short" })
    .formatToParts(0)
    .find((part, i, parts) => part.type === "literal" && parts[i + 1].type === "dayPeriod")?.value || "";

const date = new Date("1886-05-01T14:12:47Z");
const dateOptions = [
  ["full", "Saturday, May 1, 1886"],
  ["long", "May 1, 1886"],
  ["medium", "May 1, 1886"],
  ["short", "5/1/86"],
];

const timeOptions = [
  ["full", `2:12:47${usDayPeriodSpace}PM Coordinated Universal Time`, "14:12:47 Coordinated Universal Time"],
  ["long", `2:12:47${usDayPeriodSpace}PM UTC`, "14:12:47 UTC"],
  ["medium", `2:12:47${usDayPeriodSpace}PM`, "14:12:47"],
  ["short", `2:12${usDayPeriodSpace}PM`, "14:12"],
];

const options12 = [
  { "hour12": true },
  { "hourCycle": "h11" },
  { "hourCycle": "h12" },
  { "hourCycle": "h23", "hour12": true },
  { "hourCycle": "h24", "hour12": true },
];

const options24 = [
  { "hour12": false },
  { "hourCycle": "h23" },
  { "hourCycle": "h24" },
  { "hourCycle": "h11", "hour12": false },
  { "hourCycle": "h12", "hour12": false },
];

for (const [dateStyle, _expected] of dateOptions) {
  const dtf = new Intl.DateTimeFormat("en-US", {
    dateStyle,
    timeZone: "UTC",
  });

  const dateString = dtf.format(date);
  print(dateString);
}

// CHECK: Saturday, May 1, 1886
// CHECK-NEXT: May 1, 1886
// CHECK-NEXT: May 1, 1886
// CHECK-NEXT: 5/1/86

for (const [timeStyle, expected12, _expected24] of timeOptions) {
  const check = (locale, options, expected) => {
    const dtf = new Intl.DateTimeFormat(locale, {
      timeStyle,
      timeZone: "UTC",
      ...options
    });

    const dateString = dtf.format(date);
    if (dateString !== expected)
      throw new Error(`Result for ${timeStyle} in ${locale} with ${JSON.stringify(options)}`);
  };

  check("en-US", {}, expected12);
  check("en-US-u-hc-h11", {}, expected12);
  check("en-US-u-hc-h12", {}, expected12);
  // TODO: Apple does not allow overriding the hour cycle
  // for timeStyle, so this will always match the "base"
  // locale, en-US. In the future we may be able to workaround
  // this by using attributed strings with Swift's DateFormat
  // check("en-US-u-hc-h23", {}, expected24);
  check("en-US-u-hc-h23", {}, expected12);
  // check("en-US-u-hc-h24", {}, expected24);
  check("en-US-u-hc-h24", {}, expected12);

  for (const hourOptions of options12) {
    check("en-US", hourOptions, expected12);
    check("en-US-u-hc-h11", hourOptions, expected12);
    check("en-US-u-hc-h12", hourOptions, expected12);
    check("en-US-u-hc-h23", hourOptions, expected12);
    check("en-US-u-hc-h24", hourOptions, expected12);
  }

  for (const hourOptions of options24) {
    // TODO: Apple does not allow overriding the hour cycle
    // for timeStyle, so this will always match the "base"
    // locale, en-US. In the future we may be able to workaround
    // this by using attributed strings with Swift's DateFormat
    // check("en-US", hourOptions, expected24);
    // check("en-US-u-hc-h11", hourOptions, expected24);
    // check("en-US-u-hc-h12", hourOptions, expected24);
    // check("en-US-u-hc-h23", hourOptions, expected24);
    // check("en-US-u-hc-h24", hourOptions, expected24);

    check("en-US", hourOptions, expected12);
    check("en-US-u-hc-h11", hourOptions, expected12);
    check("en-US-u-hc-h12", hourOptions, expected12);
    check("en-US-u-hc-h23", hourOptions, expected12);
    check("en-US-u-hc-h24", hourOptions, expected12);
  }

  print(`success: ${timeStyle}`)
}

// CHECK: success: full
// CHECK-NEXT: success: long
// CHECK-NEXT: success: medium
// CHECK-NEXT: success: short


for (const [dateStyle, expectedDate] of dateOptions) {
  for (const [timeStyle, expectedTime] of timeOptions) {
    const dtf = new Intl.DateTimeFormat("en-US", {
      dateStyle,
      timeStyle,
      timeZone: "UTC",
    });
    const result1 = [expectedDate, ", ", expectedTime].join("");
    const result2 = [expectedDate, " at ", expectedTime].join("");

    const dateString = dtf.format(date);
    if (![result1, result2].includes(dateString))
      throw new Error(`Result for date=${dateStyle} and time=${timeStyle}`);

    print(`success: dateStyle=${dateStyle}, timeStyle=${timeStyle}`)
  }
}

// CHECK: success: dateStyle=full, timeStyle=full
// CHECK-NEXT: success: dateStyle=full, timeStyle=long
// CHECK-NEXT: success: dateStyle=full, timeStyle=medium
// CHECK-NEXT: success: dateStyle=full, timeStyle=short
// CHECK-NEXT: success: dateStyle=long, timeStyle=full
// CHECK-NEXT: success: dateStyle=long, timeStyle=long
// CHECK-NEXT: success: dateStyle=long, timeStyle=medium
// CHECK-NEXT: success: dateStyle=long, timeStyle=short
// CHECK-NEXT: success: dateStyle=medium, timeStyle=full
// CHECK-NEXT: success: dateStyle=medium, timeStyle=long
// CHECK-NEXT: success: dateStyle=medium, timeStyle=medium
// CHECK-NEXT: success: dateStyle=medium, timeStyle=short
// CHECK-NEXT: success: dateStyle=short, timeStyle=full
// CHECK-NEXT: success: dateStyle=short, timeStyle=long
// CHECK-NEXT: success: dateStyle=short, timeStyle=medium
// CHECK-NEXT: success: dateStyle=short, timeStyle=short
