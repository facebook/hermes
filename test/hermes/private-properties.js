/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print('private properties');
// CHECK-LABEL: private properties

// Trivial case.
(function () {
  class A {
    #f1 = 10;
  }
  let inst = new A();
  print(Object.getOwnPropertySymbols(inst).length);
// CHECK-NEXT: 0
})();

// Check for existence.
(function () {
  class B {
    #f1 = 10;
    static hasPrivate(o) {
      return #f1 in o;
    }
  }
  let inst = new B();
  print(B.hasPrivate({}));
// CHECK-NEXT: false
  print(B.hasPrivate(inst));
// CHECK-NEXT: true
})();

class Forwarder {
  constructor(o) {
    return o;
  }
}

// Double-initialization throws.
(function () {
  class C extends Forwarder {
    #f1 = 10;
  }
  let inst = new C();
  try {
    new C(inst);
    print("fail");
  } catch (e) {
    print(e.constructor.name);
  }
// CHECK-NEXT: TypeError
})();

// Double-initialization throws only after side effects of value initializer.
(function () {
  var calls = 0;
  function foo() {
    calls++;
    if (calls >= 2) {
      throw new Error();
    }
  }
  class C extends Forwarder {
    #f1 = foo();
  }
  let inst = new C();
  try {
    // This does not throw a double-initialization error, but instead the error from `foo` is thrown.
    new C(inst);
  } catch (e) {
    print(e.constructor.name);
  }
// CHECK-NEXT: Error
})();
