/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes %s -lazy | %FileCheck --match-full-lines %s

print("super properties");
// CHECK-LABEL: super properties

const obj1 = { prop: 1 };
const obj2 = { prop: 2 };

const c1 = {
  __proto__: obj1,
  printSuperProp() {
    print(super.prop);
  },
};

c1.printSuperProp();
//CHECK-NEXT: 1

const printSuperProp = c1.printSuperProp;
printSuperProp.call({});
//CHECK-NEXT: 1

const c2 = { __proto__: obj2, printSuperProp };
c2.printSuperProp();
//CHECK-NEXT: 1

Object.setPrototypeOf(c1, obj2);

c1.printSuperProp();
//CHECK-NEXT: 2

printSuperProp.call({});
//CHECK-NEXT: 2

c2.printSuperProp();
//CHECK-NEXT: 2

// Verify super propagates through all different function kinds.

const c3 = {
  __proto__: { a: 1, b: 2, c: 3 },
  normal() {
    print(super.a);
    (() => { print(super.b); })();
    (async () => { print(super.c); })();
  },
  *gen() {
    print(super.a);
    (() => { print(super.b); })();
    (async () => { print(super.c); })();
  },
  async asyncFun() {
    print(super.a);
    (() => { print(super.b); })();
    (async () => { print(super.c); })();
  }
};

c3.normal();
//CHECK-NEXT: 1
//CHECK-NEXT: 2
//CHECK-NEXT: 3
c3.gen().next();
//CHECK-NEXT: 1
//CHECK-NEXT: 2
//CHECK-NEXT: 3
c3.asyncFun();
//CHECK-NEXT: 1
//CHECK-NEXT: 2
//CHECK-NEXT: 3
