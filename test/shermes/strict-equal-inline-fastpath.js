/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -emit-c -o - %s | %FileCheck %s
// RUN: %shermes -O -Xsmall-c -emit-c -o - %s | %FileCheck --check-prefix=CHECK-SMALLC %s
// RUN: %shermes -O -exec %s | %FileCheck --check-prefix=CHECK-EXEC --match-full-lines %s

'use strict';

function strictEq(a, b) {
  'noinline';
  return a === b;
}

function strictNe(a, b) {
  'noinline';
  return a !== b;
}

var obj = {};

print(strictEq(1, 1));
print(strictEq(null, null));
print(strictEq(1, 2));
print(strictEq(undefined, null));
print(strictEq(NaN, NaN));
print(strictEq(0, -0));
print(strictEq(obj, obj));
print(strictEq({}, {}));
print(strictEq("a", "a"));
print(strictNe("a", "b"));
print(strictNe("a", "a"));

// CHECK-LABEL: static SHLegacyValue _1_strictEq(SHRuntime *shr) {
// CHECK: np0 = _sh_ljs_bool(_sh_ljs_strict_equal_inline(locals.t0, locals.t1));
// CHECK-NOT: _sh_ljs_strict_equal(
// CHECK: return np0;
//
// CHECK-LABEL: static SHLegacyValue _2_strictNe(SHRuntime *shr) {
// CHECK: np0 = _sh_ljs_bool(!_sh_ljs_strict_equal_inline(locals.t0, locals.t1));
// CHECK-NOT: _sh_ljs_strict_equal(
// CHECK: return np0;
//
// CHECK-SMALLC-LABEL: static SHLegacyValue _1_strictEq(SHRuntime *shr) {
// CHECK-SMALLC: np0 = _sh_ljs_bool(_sh_ljs_strict_equal(locals.t0, locals.t1));
// CHECK-SMALLC-NOT: _sh_ljs_strict_equal_inline
// CHECK-SMALLC: return np0;
//
// CHECK-SMALLC-LABEL: static SHLegacyValue _2_strictNe(SHRuntime *shr) {
// CHECK-SMALLC: np0 = _sh_ljs_bool(!_sh_ljs_strict_equal(locals.t0, locals.t1));
// CHECK-SMALLC-NOT: _sh_ljs_strict_equal_inline
// CHECK-SMALLC: return np0;
//
// CHECK-EXEC: true
// CHECK-EXEC-NEXT: true
// CHECK-EXEC-NEXT: false
// CHECK-EXEC-NEXT: false
// CHECK-EXEC-NEXT: false
// CHECK-EXEC-NEXT: true
// CHECK-EXEC-NEXT: true
// CHECK-EXEC-NEXT: false
// CHECK-EXEC-NEXT: true
// CHECK-EXEC-NEXT: true
// CHECK-EXEC-NEXT: false
