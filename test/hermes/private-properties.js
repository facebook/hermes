/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -test262 -O %s | %FileCheck --match-full-lines %s

print('private properties');
// CHECK-LABEL: private properties

// --- Adding properties

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

// --- Reading properties

(function () {
  class D {
    #f1 = 10;
    static printPrivate(o) {
      print(o.#f1);
    }
  }
  let inst = new D();
  D.printPrivate(inst);
// CHECK-NEXT: 10
  try {
    D.printPrivate({});
    print("fail");
  } catch (e) {
    print(e.constructor.name);
  }
// CHECK-NEXT: TypeError
})();

// --- Reading methods and accessors

// Easy, happy case.
(function () {
  class E {
    #m1() { print("#m1 called"); }
    static callMethod(o) {
      o.#m1();
    }
    get #g1() { print("#g1 called"); }
    static callGetter(o) {
      return o.#g1;
    }
  }
  E.callMethod(new E());
// CHECK-NEXT: #m1 called
  E.callGetter(new E());
// CHECK-NEXT: #g1 called
  try {
    E.callMethod({});
  } catch (e) {
    print(e.constructor.name);
// CHECK-NEXT: TypeError
  }
  try {
    E.callGetter({});
  } catch (e) {
    print(e.constructor.name);
// CHECK-NEXT: TypeError
  }
})();

// Try to read setter.
(function () {
  class F {
    set #s1(v) {}
    static tryReadS1(o) {
      return o.#s1;
    }
  }
  try {
    F.tryReadS1(new F());
  } catch (e) {
    print(e.constructor.name);
// CHECK-NEXT: TypeError
  }
})();

// Static methods and getters.
(function () {
  class G {
    static #staticM1() { print("static #m1 called"); }
    static get #staticG1() { print("static #g1 called"); }
    static callStaticMethod(o) {
      o.#staticM1();
    }
    static callStaticGetter(o) {
      return o.#staticG1;
    }
  }
  G.callStaticMethod(G);
// CHECK-NEXT: static #m1 called
  G.callStaticGetter(G);
// CHECK-NEXT: static #g1 called
  try {
    G.callStaticMethod(new G());
  } catch (e) {
    print(e.constructor.name);
// CHECK-NEXT: TypeError
  }
  try {
    G.callGetter(new G());
  } catch (e) {
    print(e.constructor.name);
// CHECK-NEXT: TypeError
  }
})();
