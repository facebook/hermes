/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O -exec %s | %FileCheck --match-full-lines %s

// NewArray reserves at most UINT16_MAX slots, so element 65535 must not use the
// unchecked dense store.

var x = 1;
var a = eval("[" + "x,".repeat(65537) + "]");
print(a.length, a[65534], a[65535], a[65536]);
// CHECK: 65537 1 1 1

var b = eval("[" + "2,".repeat(65537) + "]");
print(b.length, b[65534], b[65535], b[65536]);
// CHECK-NEXT: 65537 2 2 2
