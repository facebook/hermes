/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// Test the JStrictEqual instruction (and the inverse).
function equal(x, y) {
  if (x === y) {
    return 123;
  } else {
    return 456;
  }
}
function nequal(x, y) {
  if (x !== y) {
    return 123;
  } else {
    return 456;
  }
}
function check(x, y) {
  return [equal(x, y) === 123, nequal(x, y) === 123];
}

print(check(undefined, undefined));
// CHECK: true,false
print(check(undefined, null));
// CHECK: false,true
print(check(undefined, {}));
// CHECK: false,true
print(check(1, 1));
// CHECK: true,false
print(check(1, 2));
// CHECK-NEXT: false,true
print(check(1, NaN));
// CHECK-NEXT: false,true
print(check(NaN, NaN));
// CHECK-NEXT: false,true
print(check(NaN, 'a'));
// CHECK-NEXT: false,true
print(check('a', NaN));
// CHECK-NEXT: false,true
print(check('ab', 'ab'));
// CHECK-NEXT: true,false
globalThis.a = 'a';
print(check('ab', globalThis.a + 'b'));
// CHECK-NEXT: true,false
print(check('a', 'b'));
// CHECK-NEXT: false,true
print(check('a', 'bc'));
// CHECK-NEXT: false,true
print(check({}, 'a'));
// CHECK-NEXT: false,true
print(check('a', {}));
// CHECK-NEXT: false,true
print(check({}, {}));
// CHECK-NEXT: false,true
print(check(Math, Math));
// CHECK-NEXT: true,false
