/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print('get date time format test');
// CHECK-LABEL: get date time format test

const date = new Date(Date.UTC(2020, 0, 2, 3, 45, 0, 30));
const oldDate = new Date(Date.UTC(1952, 0, 9, 8, 4, 3, 5));

print(new Intl.DateTimeFormat('en-US').format(date));
// CHECK-NEXT: 1/2/2020

print(new Intl.DateTimeFormat('en-GB').format(date));
// CHECK-NEXT: 02/01/2020

print(new Intl.DateTimeFormat('de-DE').format(date));
// CHECK-NEXT: 2.1.2020

print(new Intl.DateTimeFormat('en-US').format(oldDate));
// CHECK-NEXT: 1/9/1952

print(new Intl.DateTimeFormat('en-GB').format(oldDate));
// CHECK-NEXT: 09/01/1952

print(new Intl.DateTimeFormat('de-DE').format(oldDate));
// CHECK-NEXT: 9.1.1952

print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'America/Los_Angeles' }).format(date));
// CHECK-NEXT: 7:45:00{{.+}}PM PST

print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'EET' }).format(date));
// CHECK-NEXT: 5:45:00{{.+}}AM GMT+2

try {
  print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'XXX' }).format(date));
  print('Succeeded');
} catch (e) {
  print('Caught', e.name, e.message);
}
// CHECK-NEXT: Caught{{.*}}

print(new Intl.DateTimeFormat('en-GB', { dateStyle: 'full', timeStyle: 'long' }).format(date));
// CHECK-NEXT: Thursday{{.+}}2 January 2020 at 03:45:00 {{GMT|UTC}}

print(new Intl.DateTimeFormat('ko-KR', { dateStyle: 'medium', timeStyle: 'medium' }).format(date));
// CHECK-NEXT: 2020. 1. 2. 오전 3:45:00

print(new Intl.DateTimeFormat('en-US', { dateStyle: 'short', timeStyle: 'medium' }).format(oldDate));
// CHECK-NEXT: 1/9/52, 8:04:03{{.+}}AM

print(new Intl.DateTimeFormat('de-DE', { dateStyle: 'full', timeStyle: 'long' }).format(oldDate));
// CHECK-NEXT: Mittwoch, 9. Januar 1952 um 08:04:03 {{GMT|UTC}}

print(new Intl.DateTimeFormat('it-IT', { dateStyle: 'long', timeStyle: 'short' }).format(oldDate));
// CHECK-NEXT: 9 gennaio 1952{{.+}}08:04

const lengthOptions = ['narrow', 'short', 'long'];
const numericOptions = ['numeric', '2-digit'];

lengthOptions.forEach((element) => print(new Intl.DateTimeFormat('en-GB', { weekday: element }).format(date)));
// CHECK-NEXT: T
// CHECK-NEXT: Thu
// CHECK-NEXT: Thursday

lengthOptions.forEach((element) => print(new Intl.DateTimeFormat('hi-IN', { weekday: element }).format(date)));
// CHECK-NEXT: गु
// CHECK-NEXT: गुरु
// CHECK-NEXT: गुरुवार

lengthOptions.forEach((element) => print(new Intl.DateTimeFormat('en-US', { era: element }).format(date)));
// CHECK-NEXT: 1{{.+}}2{{.+}}2020 A
// CHECK-NEXT: 1{{.+}}2{{.+}}2020 AD
// CHECK-NEXT: 1{{.+}}2{{.+}}2020 Anno Domini

numericOptions.forEach((element) => print(new Intl.DateTimeFormat('de-DE', { year: element }).format(date)));
// CHECK-NEXT: 2020
// CHECK-NEXT: 20

numericOptions.forEach((element) => print(new Intl.DateTimeFormat('ja-JP', { day: element }).format(date)));
// CHECK-NEXT: 2日
// CHECK-NEXT: 02日

numericOptions.forEach((element) => print(new Intl.DateTimeFormat('en-US', { minute: element }).format(date)));
// CHECK-NEXT: 45
// CHECK-NEXT: 45

numericOptions.forEach((element) =>
  print(new Intl.DateTimeFormat('en-US', { minute: element, second: element }).format(oldDate))
);
// CHECK-NEXT: 04:03
// CHECK-NEXT: 04:03

numericOptions.forEach((element) => print(new Intl.DateTimeFormat('en-GB', { second: element }).format(oldDate)));
// CHECK-NEXT: 3
// CHECK-NEXT: {{03|3}}

numericOptions.forEach((element) =>
  print(new Intl.DateTimeFormat('de-DE', { hour: element, minute: element }).format(date))
);
// CHECK-NEXT: 03:45
// CHECK-NEXT: 03:45

lengthOptions
  .concat(numericOptions)
  .forEach((element) => print(new Intl.DateTimeFormat('en-GB', { month: element }).format(date)));
// CHECK-NEXT: J
// CHECK-NEXT: Jan
// CHECK-NEXT: January
// CHECK-NEXT: 1
// CHECK-NEXT: 01

numericOptions.forEach((element) =>
  print(new Intl.DateTimeFormat('ja-JP', { hour: element, minute: element }).format(date))
);
// CHECK-NEXT: 3:45
// CHECK-NEXT: {{03:45|3:45}}

numericOptions.forEach((element) =>
  print(new Intl.DateTimeFormat('ja-JP', { hour: element, minute: element, hourCycle: 'h12' }).format(date))
);
// CHECK-NEXT: 午前3:45
// CHECK-NEXT: 午前{{03:45|3:45}}

var timeZoneNameOptions = ['short', 'shortOffset', 'shortGeneric', 'long', 'longOffset', 'longGeneric'];

for (var tzn of timeZoneNameOptions) {
  print(new Intl.DateTimeFormat('en-US', { timeZone: 'America/Los_Angeles', timeZoneName: tzn }).format(date));
}
// CHECK-NEXT: 1/1/2020, PST
// CHECK-NEXT: 1/1/2020, GMT-8
// CHECK-NEXT: 1/1/2020, PT
// CHECK-NEXT: 1/1/2020, Pacific Standard Time
// CHECK-NEXT: 1/1/2020, GMT-08:00
// CHECK-NEXT: 1/1/2020, Pacific Time

print(new Intl.DateTimeFormat('en-US').resolvedOptions().locale);
// CHECK-NEXT: en-US

print(new Intl.DateTimeFormat('zh-CN').resolvedOptions().locale);
// CHECK-NEXT: zh

print(new Intl.DateTimeFormat('en-US', { timeZone: 'Asia/SingapoRE' }).resolvedOptions().timeZone);
// CHECK-NEXT: Asia/Singapore

print(JSON.stringify(new Intl.DateTimeFormat('en-US').formatToParts(date)));
// CHECK-NEXT: [{"value":"1","type":"month"},{"value":"/","type":"literal"},{"value":"2","type":"day"},{"value":"/","type":"literal"},{"value":"2020","type":"year"}]

print(JSON.stringify(new Intl.DateTimeFormat('en-GB').formatToParts(date)));
// CHECK-NEXT: [{"value":"02","type":"day"},{"value":"/","type":"literal"},{"value":"01","type":"month"},{"value":"/","type":"literal"},{"value":"2020","type":"year"}]

print(
  JSON.stringify(
    new Intl.DateTimeFormat('en-US', {
      weekday: 'long',
      year: 'numeric',
      month: 'numeric',
      day: 'numeric',
      dayPeriod: 'long',
      hour: 'numeric',
      minute: 'numeric',
      second: 'numeric',
      fractionalSecondDigits: 3,
      hour12: true,
      timeZone: 'UTC',
    }).formatToParts(date)
  )
);
// CHECK-NEXT: [{"value":"Thursday","type":"weekday"},{{[{]"value":", ","type":"literal"[}]|[{]"value":",","type":"literal"[}],[{]"value":" ","type":"literal"[}]}},{"value":"1","type":"month"},{"value":"/","type":"literal"},{"value":"2","type":"day"},{"value":"/","type":"literal"},{"value":"2020","type":"year"},{{[{]"value":", ","type":"literal"[}]|[{]"value":",","type":"literal"[}],[{]"value":" ","type":"literal"[}]}},{"value":"3","type":"hour"},{"value":":","type":"literal"},{"value":"45","type":"minute"},{"value":":","type":"literal"},{"value":"00","type":"second"},{"value":".","type":"literal"},{"value":"030","type":"fractionalSecond"},{"value":"{{.+}}","type":"literal"},{"value":"{{at night|AM}}","type":"dayPeriod"}]

print(new Date(Date.UTC(2020, 0, 2)).toLocaleString('en-US', { weekday: 'short', timeZone: 'UTC' }));
// CHECK-NEXT: Thu

function expectException(locales, options) {
  try {
    print(new Intl.DateTimeFormat(locales, options).format(new Date()));
    print('Succeeded');
  } catch (e) {
    print('Caught', e.name, e.message);
  }
}

locales = ['es-ES', 'foobarbaz', 'es-US'];
expectException(locales, {});
// CHECK-NEXT: Caught RangeError Invalid language tag: foobarbaz

locales = 'en-US';
// Invalid localeMatcher value
let invalidOptions = { localeMatcher: 'best' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError localeMatcher value is invalid.

// Unicode extension type subtag longer than 8 characters is invalid.
invalidOptions = { calendar: 'abcdefghi' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError{{.*}}

// Unicode extension type subtag with non-letters or digits is invalid.
invalidOptions = { calendar: 'a&bc' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError{{.*}}

// Invalid hourCycle value
invalidOptions = { hourCycle: 'H23' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError hourCycle value is invalid.

// Seconds field in offset time zone id is invalid.
invalidOptions = { timeZone: '-01:00:30' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError{{.*}}

// Time zone is not a valid IANA time zone id.
invalidOptions = { timeZone: 'FOO' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError{{.*}}

const dateTimeComponents = [
  'weekday',
  'era',
  'year',
  'month',
  'day',
  'dayPeriod',
  'hour',
  'minute',
  'second',
  'timeZoneName',
];
dateTimeComponents.forEach((component) => expectException(locales, { [component]: 'full' }));
// CHECK-NEXT: Caught RangeError weekday value is invalid.
// CHECK-NEXT: Caught RangeError era value is invalid.
// CHECK-NEXT: Caught RangeError year value is invalid.
// CHECK-NEXT: Caught RangeError month value is invalid.
// CHECK-NEXT: Caught RangeError day value is invalid.
// CHECK-NEXT: Caught RangeError dayPeriod value is invalid.
// CHECK-NEXT: Caught RangeError hour value is invalid.
// CHECK-NEXT: Caught RangeError minute value is invalid.
// CHECK-NEXT: Caught RangeError second value is invalid.
// CHECK-NEXT: Caught RangeError timeZoneName value is invalid.

// fractionalSecondDigits outside of 1 - 3 is invalid.
invalidOptions = { fractionalSecondDigits: 0 };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError fractionalSecondDigits value is invalid.

invalidOptions = { fractionalSecondDigits: 4 };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError fractionalSecondDigits value is invalid.

// Invalid formatMatcher value
invalidOptions = { formatMatcher: 'best match' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError formatMatcher value is invalid.

// Invalid dateStyle value
invalidOptions = { dateStyle: 'narrow' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError dateStyle value is invalid.

// Invalid timeStyle value
invalidOptions = { timeStyle: 'numeric' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught RangeError timeStyle value is invalid.

// Specifiying date style and time style along with other date-time components is invalid.
invalidOptions = { dateStyle: 'short', hour: 'numeric' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught TypeError{{.*}}

invalidOptions = { timeStyle: 'full', hour: 'numeric' };
expectException(locales, invalidOptions);
// CHECK-NEXT: Caught TypeError{{.*}}

try {
  print(Intl.DateTimeFormat.supportedLocalesOf(['hi-IN', 'ko-KR'], { localeMatcher: 'bestfit' }));
  print('Succeeded');
} catch (e) {
  print('Caught', e.name, e.message);
}
// CHECK-NEXT: Caught RangeError localeMatcher value is invalid.
