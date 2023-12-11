/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

let a = true;
a |= a;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}binary-assign-error.js:11:1: error: ft: incompatible binary operation: |= cannot be applied to boolean and boolean
// CHECK-NEXT:a |= a;
// CHECK-NEXT:^~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
