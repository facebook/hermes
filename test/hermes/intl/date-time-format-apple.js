/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get date time format test");
// CHECK-LABEL: get date time format test

const date = new Date(Date.UTC(2020, 0, 2, 3, 45, 00, 30));
const oldDate = new Date(Date.UTC(1952, 0, 9, 8, 04, 03, 05));

print(new Intl.DateTimeFormat('en-US').format(date));
// CHECK-NEXT: 1/2/20

print(new Intl.DateTimeFormat('en-GB').format(date));
// CHECK-NEXT: 02/01/2020

print(new Intl.DateTimeFormat('de-DE').format(date));
// CHECK-NEXT: 02.01.20

print(new Intl.DateTimeFormat('en-US').format(oldDate));
// CHECK-NEXT: 1/9/52

print(new Intl.DateTimeFormat('en-GB').format(oldDate));
// CHECK-NEXT: 09/01/1952

print(new Intl.DateTimeFormat('de-DE').format(oldDate));
// CHECK-NEXT: 09.01.52

print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'PST'}).format(date));
// CHECK-NEXT: 7:45:00 PM PST

print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'EET'}).format(date));
// CHECK-NEXT: 5:45:00 AM GMT+2

try {
  print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: 'XXX'}).format(date));
    print("Succeeded");
  } catch (e) {
    print("Caught", e.name, e.message);
  }
// CHECK-NEXT: Caught{{.*}}

print(new Intl.DateTimeFormat('en-GB', { dateStyle: 'full', timeStyle: 'long' }).format(date));
// CHECK-NEXT: Thursday, 2 January 2020 at 03:45:00 GMT

print(new Intl.DateTimeFormat('ko-KR', { dateStyle: 'medium', timeStyle: 'medium' }).format(date));
// CHECK-NEXT: 2020. 1. 2. 오전 3:45:00

print(new Intl.DateTimeFormat('en-US', { dateStyle: 'short', timeStyle: 'medium' }).format(oldDate));
// CHECK-NEXT: 1/9/52, 8:04:03 AM

print(new Intl.DateTimeFormat('de-DE', { dateStyle: 'full', timeStyle: 'long' }).format(oldDate));
// CHECK-NEXT: Mittwoch, 9. Januar 1952 um 08:04:03 GMT

print(new Intl.DateTimeFormat('it-IT', { dateStyle: 'long', timeStyle: 'short' }).format(oldDate))
// CHECK-NEXT: 9 gennaio 1952, 08:04

const lengthOptions = ['narrow', 'short', 'long'];
const numericOptions = ['numeric', '2-digit'];

lengthOptions.forEach(element => print(new Intl.DateTimeFormat('en-GB', {weekday: element}).format(date)));
// CHECK-NEXT: T
// CHECK-NEXT: Thu
// CHECK-NEXT: Thursday

lengthOptions.forEach(element => print(new Intl.DateTimeFormat('hi-IN', {weekday: element}).format(date)));
// CHECK-NEXT: गु
// CHECK-NEXT: गुरु
// CHECK-NEXT: गुरुवार

lengthOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {era: element}).format(date)));
// CHECK-NEXT: A
// CHECK-NEXT: AD
// CHECK-NEXT: Anno Domini

numericOptions.forEach(element => print(new Intl.DateTimeFormat('de-DE', {year: element}).format(date)));
// CHECK-NEXT: 2020
// CHECK-NEXT: 20

numericOptions.forEach(element => print(new Intl.DateTimeFormat('ja-JP', {day: element}).format(date)));
// CHECK-NEXT: 2日
// CHECK-NEXT: 02日

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {minute: element}).format(date)));
// CHECK-NEXT: 45
// CHECK-NEXT: 45

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {minute: element, second: element}).format(oldDate)));
// CHECK-NEXT: 04:03
// CHECK-NEXT: 04:03

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-GB', {second: element}).format(oldDate)));
// CHECK-NEXT: 3
// CHECK-NEXT: 3

lengthOptions.concat(numericOptions).forEach(element => print(new Intl.DateTimeFormat('en-GB', {month: element}).format(date)));
// CHECK-NEXT: J
// CHECK-NEXT: Jan
// CHECK-NEXT: January
// CHECK-NEXT: 1
// CHECK-NEXT: 01

lengthOptions.slice(1).forEach(element => print(new Intl.DateTimeFormat('en-GB', {timeZoneName: element}).format(date)));
// CHECK-NEXT: GMT
// CHECK-NEXT: Greenwich Mean Time

numericOptions.forEach(element => print(new Intl.DateTimeFormat('ja-JP', {hour: element, minute: element}).format(date)));
// CHECK-NEXT: 午前3:45
// CHECK-NEXT: 午前3:45

print(new Intl.DateTimeFormat('en-US').resolvedOptions().locale);
// CHECK-NEXT: en-US

print(new Intl.DateTimeFormat('zh-CN').resolvedOptions().locale);
// CHECK-NEXT: zh

print(new Intl.DateTimeFormat('en-US').resolvedOptions().numberingSystem);
// CHECK-NEXT: undefined

print(new Intl.DateTimeFormat('en-US', { timeZone: 'SGT'}).resolvedOptions().timeZone);
// CHECK-NEXT: SGT

print(JSON.stringify(new Intl.DateTimeFormat('en-US').formatToParts(date)));
// CHECK-NEXT: [{"value":"1","type":"month"},{"value":"/","type":"literal"},{"value":"2","type":"day"},{"value":"/","type":"literal"},{"value":"20","type":"year"}]

print(JSON.stringify(new Intl.DateTimeFormat('en-GB').formatToParts(date)));
// CHECK-NEXT: [{"value":"02","type":"day"},{"value":"/","type":"literal"},{"value":"01","type":"month"},{"value":"/","type":"literal"},{"value":"2020","type":"year"}]

print(JSON.stringify(new Intl.DateTimeFormat('en-US', {weekday: 'long',
  year: 'numeric',
  month: 'numeric',
  day: 'numeric',
  hour: 'numeric',
  minute: 'numeric',
  second: 'numeric',
  fractionalSecondDigits: 3,
  hour12: true,
  timeZone: 'UTC'
}).formatToParts(new Date(Date.UTC(2020, 0, 2, 3, 45, 00, 30)))));
// CHECK-NEXT: [{"value":"Thursday","type":"weekday"},{"value":",","type":"literal"},{"value":" ","type":"literal"},{"value":"1","type":"month"},{"value":"/","type":"literal"},{"value":"2","type":"day"},{"value":"/","type":"literal"},{"value":"2020","type":"year"},{"value":",","type":"literal"},{"value":" ","type":"literal"},{"value":"3","type":"hour"},{"value":":","type":"literal"},{"value":"45","type":"minute"},{"value":":","type":"literal"},{"value":"00","type":"second"},{"value":".","type":"literal"},{"value":"030","type":"fractionalSecond"},{"value":" ","type":"literal"},{"value":"AM","type":"dayPeriod"}]
