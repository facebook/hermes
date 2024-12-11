/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes %s -exec | %FileCheck --match-full-lines %s

print("class fields");
// CHECK-LABEL: class fields

function dumpProps(o) {
  for (k of Object.keys(o)){
    print(k, ":", o[k]);
  }
}

// Simple, easy case.
(function () {
  class P1 {
    f1 = 10;
// These outputs are produced by the dumpProps calls, below.
// CHECK-NEXT: f1 : 10
    f2 = 20;
// CHECK-NEXT: f2 : 20
    static f1 = 30;
// CHECK-NEXT: f1 : 30
    static f2 = 40;
// CHECK-NEXT: f2 : 40
  }
  dumpProps(new P1());
  dumpProps(P1);
})();

// Correct evaluation order of computed keys and static field values.
(function () {
  var x = 0;
  class P1 {
    static f1 = "base";
// CHECK-NEXT: f1 : base
    static p1 = 40;
// CHECK-NEXT: p1 : 40
    static p2 = P1.p1 + 1;
// CHECK-NEXT: p2 : 41
    // Computed keys all evaluated before static field values.
    static ["x" + (++x)] = (++x);
// CHECK-NEXT: x1 : 4
    static ["method" + (++x)](){}
    static ["x" + (++x)] = (++x);
// CHECK-NEXT: x3 : 5
    // `this` in computed keys does not resolve to the class.
    static ["y" + this.p1 + 1] = (++x);
// CHECK-NEXT: yundefined1 : 6
  }
  dumpProps(P1);
  print(Object.getOwnPropertyNames(P1).includes("method2"));
// CHECK-NEXT: true
})();

// `this` in field initializer value.
(function () {
  var x = 0;
  class P1 {
    f1 = 10;
// CHECK-NEXT: f1 : 10
    f2 = this.f1 + 10;
// CHECK-NEXT: f2 : 20
    static f1 = 30;
// CHECK-NEXT: f1 : 30
    static f2 = this.f1 + 10;
// CHECK-NEXT: f2 : 40
  }
  dumpProps(new P1());
  dumpProps(P1);
})();



// "prototype" is protected at runtime.
(function () {
  function otype() {
    return "otype";
  }
  try {
    class P1 {
      static ["prot" + otype()] = {};
    }
    print("Incorrectly ran");
  } catch(e) {
    print("Correctly caught");
// CHECK-NEXT: Correctly caught
  }
})();

// `this` in (static) field values.
(function () {
  class P1 {
    f1 = 10;
// CHECK-NEXT: f1 : 10
    f2 = this.f1 + 10;
// CHECK-NEXT: f2 : 20
    static f1 = 30;
// CHECK-NEXT: f1 : 30
    static f2 = this.f1 + 10;
// CHECK-NEXT: f2 : 40
  }
  dumpProps(new P1());
  dumpProps(P1);
})();

// Super properties in (static) field values.
(function () {
  class P1 {
    func() {
      return "instance func";
    }
    static func() {
      return "static func";
    }
  }
  class C1 extends P1 {
    f1 = super.func();
// CHECK-NEXT: f1 : instance func
    static f1 = super.func();
// CHECK-NEXT: f1 : static func
  }
  dumpProps(new C1());
  dumpProps(C1);
})();

// Re-wire `super` after class evaluation.
(function () {
  class P1 {
    get prop() {
      return "P1";
    }
  }
  class P2 {
    get prop() {
      return "P2";
    }
  }
  class C1 extends P1 {
    f1 = super.prop;
  }
  print(new C1().f1);
// CHECK-NEXT: P1

  Object.setPrototypeOf(C1.prototype, P2.prototype);
  print(new C1().f1);
// CHECK-NEXT: P2
})();

// Arrow functions inside value initializers get correct captured state.
(function () {
  class A {
    x = 1;
// These outputs are produced by the dumpProps calls, below.
// CHECK-NEXT: x : 1
    y = (() => {
      return this.x + 1;
    })();
// CHECK-NEXT: y : 2
    z = (() => {
      return new.target;
    })();
// CHECK-NEXT: z : undefined

    static a = 3;
// CHECK-NEXT: a : 3
    static b = (() => {
      return this.a + 1;
    })();
// CHECK-NEXT: b : 4
    static c = (() => {
      return new.target;
    })();
// CHECK-NEXT: c : undefined
  };
  dumpProps(new A());
  dumpProps(A);
})();

// Computed property names are fully evaluated via ToPropertyKey during
// class definition evaluation.
(function () {
  var o1 = {};
  o1[Symbol.toPrimitive] = function() { return "prop1"; };
  class C {
    [o1] = "val";
  }
  o1[Symbol.toPrimitive] = function() { return "prop2"; };
  let inst = new C();
  print(Object.keys(inst)[0]);
// CHECK-NEXT: prop1
})();

// Use CreateDataPropertyOrThrow when adding fields. Importantly, this calls into DefineOwnProperty. 
(function () {
  class A {
    constructor(o) {
      return o;
    }
  }
  class B extends A {
    f1 = 10;
  }
  try {
    // Non-writable on own.
    let obj = {};
    Object.defineProperty(obj, 'f1', {
      value: 'Hello',
      writable: false
    });
    new B(obj);
    print("FAIL");
  } catch (e) {
    print("PASS");
// CHECK-NEXT: PASS
  }
  try {
    // Non-writable on prototype chain.
    let parent = {};
    let obj = Object.create(parent);
    Object.defineProperty(parent, 'f1', {
      value: 'Hello',
      writable: false
    });
    new B(obj);
    print("PASS");
// CHECK-NEXT: PASS
  } catch (e) {
    print("FAIL");
  }
})();
