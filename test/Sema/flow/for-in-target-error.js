/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// for-in always produces a string key, so a target that cannot hold one is
// rejected, in both the declaration and the expression form.

let obj: any = {a: 1};
let n: number = 0;

for (const k: number in obj) {}
for (n in obj) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}for-in-target-error.js:16:12: error: ft: incompatible for-in target type: cannot implicitly cast from string to number
// CHECK-NEXT:for (const k: number in obj) {}
// CHECK-NEXT:           ^~~~~~~~~
// CHECK-NEXT:{{.*}}for-in-target-error.js:17:6: error: ft: incompatible for-in target type: cannot implicitly cast from string to number
// CHECK-NEXT:for (n in obj) {}
// CHECK-NEXT:     ^
// CHECK-NEXT:Emitted 2 errors. exiting.
