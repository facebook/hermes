/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="EST5EDT,M3.2.0/2,M11.1.0/2" %hermes -O %s | %FileCheck --match-full-lines %s
// XFAIL: windows
"use strict";

// These tests ensure that local to UTC time conversion yields consistent result
// with other engines, in the absence of ICU data. We use a POSIX TZ string
// matching America/New_York (post-2007 US DST rules) to test both the
// transition points from and to DST. Using a POSIX string instead of the IANA
// name "America/New_York" makes the test work on systems without the IANA
// timezone database installed.
// Note that these tests may fail if the time zone offset (without DST) changes
// in the future (which never happened in the past 20 years).
print("Local to UTC time");
// CHECK-LABEL: Local to UTC time

// Transition to DST
print(new Date(2013, 2, 10, 0));
// CHECK-NEXT: Sun Mar 10 2013 00:00:00 GMT-0500
print(new Date(2013, 2, 10, 1));
// CHECK-NEXT: Sun Mar 10 2013 01:00:00 GMT-0500
print(new Date(2013, 2, 10, 2));
// CHECK-NEXT: Sun Mar 10 2013 03:00:00 GMT-0400
print(new Date(2013, 2, 10, 3));
// CHECK-NEXT: Sun Mar 10 2013 03:00:00 GMT-0400

// Transition from DST
print(new Date(2013, 10, 3, 0));
// CHECK-NEXT: Sun Nov 03 2013 00:00:00 GMT-0400
print(new Date(2013, 10, 3, 1));
// CHECK-NEXT: Sun Nov 03 2013 01:00:00 GMT-0400
print(new Date(2013, 10, 3, 2));
// CHECK-NEXT: Sun Nov 03 2013 02:00:00 GMT-0500
print(new Date(2013, 10, 3, 3));
// CHECK-NEXT: Sun Nov 03 2013 03:00:00 GMT-0500

