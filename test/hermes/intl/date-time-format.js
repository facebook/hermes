/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get date time format test");
// CHECK-LABEL: get date time format test

const date = new Date(Date.UTC(2020, 0, 2, 3, 45, 00, 30));
const oldDate = new Date(Date.UTC(1952, 0, 9, 8, 04, 03, 05));

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

print(new Intl.DateTimeFormat('en-GB', { dateStyle: 'full', timeStyle: 'long' }).format(date));
// CHECK-NEXT: Thursday, 2 January 2020 at 03:45:00 GMT

print(new Intl.DateTimeFormat('ko-KR', { dateStyle: 'medium', timeStyle: 'medium' }).format(date));
// CHECK-NEXT: 2020. 1. 2. 오전 3:45:00

print(new Intl.DateTimeFormat('en-US', { dateStyle: 'short', timeStyle: 'medium' }).format(oldDate));
// CHECK-NEXT: 1/9/52, 8:04:03 AM

print(new Intl.DateTimeFormat('de-DE', { dateStyle: 'full', timeStyle: 'long' }).format(oldDate));
// CHECK-NEXT: Mittwoch, 9. Januar 1952 um 08:04:03 GMT

print(new Intl.DateTimeFormat('it-IT', { dateStyle: 'long', timeStyle: 'short' }).format(oldDate))
// CHECK-NEXT: 9 gennaio 1952 08:04

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
// CHECK-NEXT: 1 2, 2020 A
// CHECK-NEXT: 1 2, 2020 AD
// CHECK-NEXT: 1 2, 2020 Anno Domini

numericOptions.forEach(element => print(new Intl.DateTimeFormat('de-DE', {year: element}).format(date)));
// CHECK-NEXT: 2020
// CHECK-NEXT: 20

numericOptions.forEach(element => print(new Intl.DateTimeFormat('jp-JP', {day: element}).format(date)));
// CHECK-NEXT: 2
// CHECK-NEXT: 02

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
// CHECK-NEXT: 02/01/2020, GMT
// CHECK-NEXT: 02/01/2020, Greenwich Mean Time

numericOptions.forEach(element => print(new Intl.DateTimeFormat('jp-JP', {hour: element, minute: element}).format(date)));
// CHECK-NEXT: 03:45
// CHECK-NEXT: 03:45
