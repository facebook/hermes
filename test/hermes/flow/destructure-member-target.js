/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

// A destructuring or for-of target reaches IRGen through createLRef rather
// than genExpression, so emitMemberStore must narrow the stored value. Cover
// one case per selected store kind: class field, exact object field, static
// field and fast array element.

class A {
  static s: number = 0;
  f: number = 0;
}

let inst: A = new A();
let obj: {x: number} = {x: 0};
let arr: number[] = [1, 2, 3];
let src: any = {p: 7};
let bad: any = {p: 'str'};

({p: inst.f} = {p: 2});
print('typed source', inst.f);
// CHECK: typed source 2

({p: inst.f} = src);
({p: obj.x} = src);
({p: A.s} = src);
({p: arr[0]} = src);
print('any source', inst.f, obj.x, A.s, arr[0]);
// CHECK-NEXT: any source 7 7 7 7

for (obj.x of arr) {}
print('for-of', obj.x);
// CHECK-NEXT: for-of 3

// A value that does not fit the target's type throws rather than corrupting
// the typed slot.
try {
  ({p: obj.x} = bad);
  print('no throw');
} catch (e) {
  print('caught', e.message, obj.x);
}
// CHECK-NEXT: caught Checked cast failed 3
