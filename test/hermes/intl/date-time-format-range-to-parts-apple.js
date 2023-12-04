/**
 * Copyright (C) 2019 the V8 project authors, Igalia S.L. All rights reserved.
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Based on test262/test/intl402/DateTimeFormat/prototype/formatRangeToParts/en-US.js
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

const usDateRangeSeparator = new Intl.DateTimeFormat("en-US", {
  dateStyle: "short",
})
  .formatRangeToParts(1 * 86400 * 1000, 366 * 86400 * 1000)
  .find((part) => part.type === "literal" && part.source === "shared").value;

const date1 = new Date("2019-01-03T00:00:00");
const date2 = new Date("2019-01-05T00:00:00");
const date3 = new Date("2019-03-04T00:00:00");
const date4 = new Date("2020-03-04T00:00:00");

let dtf = new Intl.DateTimeFormat("en-US");
print(JSON.stringify(dtf.formatRangeToParts(date1, date1)));
// CHECK: [{"type":"month","value":"1","source":"shared"},{"type":"literal","value":"/","source":"shared"},{"type":"day","value":"3","source":"shared"},{"type":"literal","value":"/","source":"shared"},{"type":"year","value":"2019","source":"shared"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date2)));
// CHECK: [{"type":"month","value":"1","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"1","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"5","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date3)));
// CHECK: [{"type":"month","value":"1","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"3","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date4)));
// CHECK: [{"type":"month","value":"1","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"3","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date2, date3)));
// CHECK: [{"type":"month","value":"1","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"5","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"3","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date2, date4)));
// CHECK: [{"type":"month","value":"1","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"5","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"3","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date3, date4)));
// CHECK: [{"type":"month","value":"3","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"day","value":"4","source":"startRange"},{"type":"literal","value":"/","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"3","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":"/","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]

dtf = new Intl.DateTimeFormat("en-US", {
  year: "numeric",
  month: "short",
  day: "numeric",
});
print(JSON.stringify(dtf.formatRangeToParts(date1, date1)));
// CHECK: [{"type":"month","value":"Jan","source":"shared"},{"type":"literal","value":" ","source":"shared"},{"type":"day","value":"3","source":"shared"},{"type":"literal","value":", ","source":"shared"},{"type":"year","value":"2019","source":"shared"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date2)));
// CHECK: [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Jan","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"5","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]
// We don't yet support formatting to parts for ranges which share elements, in general.
// [{"type":"month","value":"Jan","source":"shared"},{"type":"literal","value":" ","source":"shared"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"day","value":"5","source":"endRange"},{"type":"literal","value":", ","source":"shared"},{"type":"year","value":"2019","source":"shared"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date3)));
// CHECK: [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]
// [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"shared"},{"type":"year","value":"2019","source":"shared"}]

print(JSON.stringify(dtf.formatRangeToParts(date1, date4)));
// CHECK: [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"3","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date2, date3)));
// CHECK: [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"5","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2019","source":"endRange"}]
// [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"5","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"shared"},{"type":"year","value":"2019","source":"shared"}]

print(JSON.stringify(dtf.formatRangeToParts(date2, date4)));
// CHECK: [{"type":"month","value":"Jan","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"5","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]

print(JSON.stringify(dtf.formatRangeToParts(date3, date4)));
// CHECK: [{"type":"month","value":"Mar","source":"startRange"},{"type":"literal","value":" ","source":"startRange"},{"type":"day","value":"4","source":"startRange"},{"type":"literal","value":", ","source":"startRange"},{"type":"year","value":"2019","source":"startRange"},{"type":"literal","value":" – ","source":"shared"},{"type":"month","value":"Mar","source":"endRange"},{"type":"literal","value":" ","source":"endRange"},{"type":"day","value":"4","source":"endRange"},{"type":"literal","value":", ","source":"endRange"},{"type":"year","value":"2020","source":"endRange"}]
