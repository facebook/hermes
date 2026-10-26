/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="MST+7" %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: TZ="MST+7" %shermes -exec %s | %FileCheck --match-full-lines %s
"use strict";

// Like the Date constructor, Date.parse returns NaN for times outside the
// TimeClip range, whether the string is in UTC or local time (-07:00 here).
print('Date.parse');
// CHECK-LABEL: Date.parse
print(Date.parse('+275760-09-13T00:00:00.000Z'));
// CHECK-NEXT: 8640000000000000
print(Date.parse('+275760-09-13T00:00:00.001Z'));
// CHECK-NEXT: NaN
print(Date.parse('-271821-04-20T00:00:00.000Z'));
// CHECK-NEXT: -8640000000000000
print(Date.parse('-271821-04-19T23:59:59.999Z'));
// CHECK-NEXT: NaN
print(Date.parse('+275760-09-12T17:00:00.000'));
// CHECK-NEXT: 8640000000000000
print(Date.parse('+275760-09-12T17:00:00.001'));
// CHECK-NEXT: NaN
print(Date.parse('-271821-04-19T16:59:59.999'));
// CHECK-NEXT: NaN
