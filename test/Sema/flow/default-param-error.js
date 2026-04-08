/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Type mismatch: string default for number param.
function badDefault(x: number = "hello"): void {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}default-param-error.js:11:33: error: ft: incompatible default argument type
// CHECK-NEXT:function badDefault(x: number = "hello"): void {}
// CHECK-NEXT:                                ^~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
