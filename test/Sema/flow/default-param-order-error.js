/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Default param followed by required param.
function badOrder(x: number = 1, y: number): void {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}default-param-order-error.js:11:34: error: ft: optional params must be last
// CHECK-NEXT:function badOrder(x: number = 1, y: number): void {}
// CHECK-NEXT:                                 ^~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
