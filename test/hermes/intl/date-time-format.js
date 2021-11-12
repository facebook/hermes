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

print(new Intl.DateTimeFormat('en-US').format(date));
// CHECK-NEXT: 1/2/2020

print(new Intl.DateTimeFormat('en-GB').format(date));
// CHECK-NEXT: 02/01/2020

print(new Intl.DateTimeFormat('de-DE').format(date));
// CHECK-NEXT: 2.1.2020

print(new Intl.DateTimeFormat('en-GB', { dateStyle: 'full', timeStyle: 'long' }).format(date));
// CHECK-NEXT: Thursday, 2 January 2020 at 01:45:00 GMT

const lengthOptions = ['narrow', 'short', 'long'];
const numericOptions = ['numeric', '2-digit'];

lengthOptions.forEach(element => print(new Intl.DateTimeFormat('en-GB', {weekday: element}).format(date)));
// CHECK-NEXT: T,Thu,Thursday

lengthOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {era: element}).format(date)));
// CHECK-NEXT: 1 2, 2020 A,1 2, 2020 AD,1 2, 2020 Anno Domini

numericOptions.forEach(element => print(new Intl.DateTimeFormat('de-DE', {year: element}).format(date)));
// CHECK-NEXT: 2020,20

numericOptions.forEach(element => print(new Intl.DateTimeFormat('jp-JP', {day: element}).format(date)));
// CHECK-NEXT: 2,02

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {hour: element}).format(date)));
// CHECK-NEXT: 3 AM,03 AM

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-US', {minute: element}).format(date)));
// CHECK-NEXT: 45,45

numericOptions.forEach(element => print(new Intl.DateTimeFormat('en-GB', {second: element}).format(date)));
// CHECK-NEXT: 0,0

lengthOptions.concat(numericOptions).forEach(element => print(new Intl.DateTimeFormat('en-GB', {month: element}).format(date)));
// CHECK-NEXT: J,Jan,January,1,01

lengthOptions.slice(1).forEach(element => print(new Intl.DateTimeFormat('en-GB', {timeZoneName: element}).format(date)));
// CHECK-NEXT: 02/01/2020, GMT,02/01/2020, Greenwich Mean Time
