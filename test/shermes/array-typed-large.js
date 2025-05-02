/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s

'use strict';

var large: number[] = [0];
print(large[0]);
// CHECK-LABEL: 0

for (var i = 0; i < 20; ++i) {
    large.push(...large);
}

print(large.length);
// CHECK-NEXT: 1048576

// 1M elements needs at least 4MB storage if using HV32, which isn't possible
// without large allocation.
large.push(223344);
print(large[large.length - 1]);
// CHECK-NEXT: 223344

