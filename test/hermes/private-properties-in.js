/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print("private in");
// CHECK-LABEL: private in

// Simple case.
(function () {
  class A {
    #f1;
    static hasPrivateProp(o) {
      return #f1 in o;
    }
  }
  let inst = new A();
  print(A.hasPrivateProp(inst));
// CHECK-NEXT: true
  print(A.hasPrivateProp({}));
// CHECK-NEXT: false
  let childInst = Object.create(inst);
  // Searching for a private property does not travel along the prototype chain.
  print(A.hasPrivateProp(childInst));
// CHECK-NEXT: false
})();

// Test private names from enclosing scope.
(function () {
  class Outer {
    #f1;
    static getInner() {
      return class {
        static hasOuterPrivate(o) {
          return #f1 in o;
        }
      };
    }
  }
  let Inner = Outer.getInner();
  let outerInst = new Outer();
  print(Inner.hasOuterPrivate(outerInst));
// CHECK-NEXT: true
})();

// Proxy can be branded and does not trigger any traps.
(function () {
  class A {
    constructor(o) {
      return o;
    }
  }
  class B extends A {
    #f1;
    static hasPrivateProp(o) {
      return #f1 in o;
    }
  }
  let inst = new B(new Proxy({}, {
    has(target, key) {
      print("has trap called:", key);
      return false;
    }
  }));
  "b" in inst;
// CHECK-NEXT: has trap called: b
  // This does not trigger a proxy trap.
  print(B.hasPrivateProp(inst));
// CHECK-NEXT: true
})();
