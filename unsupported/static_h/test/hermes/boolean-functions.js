/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print('Number');
// CHECK-LABEL: Number
print(Boolean());
// CHECK-NEXT: false
print(Boolean(true), Boolean(false));
// CHECK-NEXT: true false
print(new Boolean(true), new Boolean(false));
// CHECK-NEXT: true false
print(true.toString(), false.toString());
// CHECK-NEXT: true false
print(true.valueOf(), false.valueOf());
// CHECK-NEXT: true false
