/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Leading void param is omittable, but trailing required param is not.
function bad1(x: void, y: number): void {}
bad1();

// Trailing void param is omittable, but leading required param is not.
function bad2(x: number, y: void): void {}
bad2();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}void-param-optional-error.js:12:1: error: ft: function expects 2 arguments, but 0 supplied
// CHECK-NEXT:bad1();
// CHECK-NEXT:^~~~~~
// CHECK-NEXT:{{.*}}void-param-optional-error.js:16:1: error: ft: function expects at least 1 arguments, but 0 supplied
// CHECK-NEXT:bad2();
// CHECK-NEXT:^~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
