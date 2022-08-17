/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="MST+7MDT,M3.2.0/2,M11.1.0/2" %hermes -O %s | %FileCheck --match-full-lines %s
"use strict";

// Tests are being run under Mountain Time, with DST changes enabled (TZ var).
// Makes sure that local time (MST/MDT) is used when no time zone is provided
print("Default to local time");
// CHECK-LABEL: Default to local time
print(Date.parse("Wed May 26 2021 13:59:26"), Date.parse("Wed May 26 2021 13:59:26 MST"));
print(Date.parse("2021-05-26 13:59:26"), Date.parse("2021-05-26 13:59:26-07:00"));
// CHECK-NEXT: 1622059166000 1622062766000
// CHECK-NEXT: 1622059166000 1622062766000
print(Date.parse("Wed May 26 2021 13:59:26") === Date.parse("Wed May 26 2021 13:59:26 MST"));
print(Date.parse("2021-05-26 13:59:26") === Date.parse("2021-05-26 13:59:26-07:00"));
// CHECK-NEXT: false
// CHECK-NEXT: false
print(Date.parse("Wed May 26 2021 13:59:26"), Date.parse("Wed May 26 2021 13:59:26 MDT"));
print(Date.parse("2021-05-26 13:59:26"), Date.parse("2021-05-26 13:59:26-06:00"));
// CHECK-NEXT: 1622059166000 1622059166000
// CHECK-NEXT: 1622059166000 1622059166000
print(Date.parse("Wed May 26 2021 13:59:26") === Date.parse("Wed May 26 2021 13:59:26 MDT"));
print(Date.parse("2021-05-26 13:59:26") === Date.parse("2021-05-26 13:59:26-06:00"));
// CHECK-NEXT: true
// CHECK-NEXT: true

print(Date.parse("Sat Dec 26 2020 13:59:26"), Date.parse("Sat Dec 26 2020 13:59:26 MST"));
print(Date.parse("2020-12-26 13:59:26"), Date.parse("2020-12-26 13:59:26-07:00"));
// CHECK-NEXT: 1609016366000 1609016366000
// CHECK-NEXT: 1609016366000 1609016366000
print(Date.parse("Sat Dec 26 2020 13:59:26") === Date.parse("Sat Dec 26 2020 13:59:26 MST"));
print(Date.parse("2020-12-26 13:59:26") === Date.parse("2020-12-26 13:59:26-07:00"));
// CHECK-NEXT: true
// CHECK-NEXT: true
print(Date.parse("Sat Dec 26 2020 13:59:26"), Date.parse("Sat Dec 26 2020 13:59:26 MDT"));
print(Date.parse("2020-12-26 13:59:26"), Date.parse("2020-12-26 13:59:26-06:00"));
// CHECK-NEXT: 1609016366000 1609012766000
// CHECK-NEXT: 1609016366000 1609012766000
print(Date.parse("Sat Dec 26 2020 13:59:26") === Date.parse("Sat Dec 26 2020 13:59:26 MDT"));
print(Date.parse("2020-12-26 13:59:26") === Date.parse("2020-12-26 13:59:26-06:00"));
// CHECK-NEXT: false
// CHECK-NEXT: false
