/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

// NewArray can reserve at most UINT16_MAX slots, so element 65535 of a larger
// literal must go through DefineOwnByIndex rather than the unchecked dense store,
// which wrote past the array storage and left the element undefined.

var x = 1;
var a = eval("[" + "x,".repeat(65537) + "]");
print(a.length, a[65534], a[65535], a[65536]);
// CHECK: 65537 1 1 1
