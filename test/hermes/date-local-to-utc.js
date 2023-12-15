/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="America/Santiago" %hermes -O %s | %FileCheck --match-full-lines %s
// XFAIL: windows
"use strict";

// These tests ensure that local to UTC time conversion yields consistent result
// with other engines, in the absence of ICU data. We use 'America/Santiago'
// here to test both the transition points from and to DST.
// Note that these tests may fail if the time zone offset (without DST) changes
// in the future (which never happened in the past 20 years).
print("Local to UTC time");
// CHECK-LABEL: Local to UTC time

// Transition to DST
print(new Date(2023, 8, 1));
// CHECK-NEXT: Fri Sep 01 2023 00:00:00 GMT-0400
print(new Date(2023, 8, 2));
// CHECK-NEXT: Sat Sep 02 2023 00:00:00 GMT-0400
print(new Date(2023, 8, 3));
// CHECK-NEXT: Sun Sep 03 2023 01:00:00 GMT-0300
print(new Date(2023, 8, 4));
// CHECK-NEXT: Mon Sep 04 2023 00:00:00 GMT-0300

// Transition from DST
print(new Date(2023, 3, 1, 22));
// CHECK-NEXT: Sat Apr 01 2023 22:00:00 GMT-0300
print(new Date(2023, 3, 1, 23));
// CHECK-NEXT: Sat Apr 01 2023 23:00:00 GMT-0300
print(new Date(2023, 3, 2, 0));
// CHECK-NEXT: Sun Apr 02 2023 00:00:00 GMT-0400
print(new Date(2023, 3, 2, 1));
// CHECK-NEXT: Sun Apr 02 2023 01:00:00 GMT-0400

