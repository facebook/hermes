/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed -O0 %s | %FileCheck --match-full-lines %s

// Typed objects in a prototype chain cached with the parent cache epoch must
// not have their shared class modified.

(function() {

class A {
  x: number;
  constructor(x: number) {
    this.x = x;
  }
  m(): number {
    return this.x;
  }
}

// Add property cache through a typed prototype.
function addProp(p: any): any {
  const o: any = Object.create(p);
  o.foo = 1;
  return o;
}
for (let i = 0; i < 3; ++i) addProp(new A(i));
const a: any = new A(10);
print(addProp(a).foo, a.m(), new A(20).m());
// CHECK: 1 10 20

// Array fast path through a typed prototype.
(Object: any).setPrototypeOf((globalThis: any).Array.prototype, new A(30));
const arr: any = (globalThis: any).Array.of(1, 2, 3);
arr.push(4);
print(arr.join(","));
// CHECK-NEXT: 1,2,3,4

})();
