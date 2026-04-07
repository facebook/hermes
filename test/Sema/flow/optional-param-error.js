/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Optional param not last in function declaration.
function declBad(x?: number, y: number): void {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}optional-param-error.js:11:30: error: ft: optional params must be last
// CHECK-NEXT:function declBad(x?: number, y: number): void {}
// CHECK-NEXT:                             ^~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
