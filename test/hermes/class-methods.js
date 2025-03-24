/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print('class methods');
// CHECK-LABEL: class methods

// Re-wire class hierarchy at runtime.
(function () {
  class P1 {
    constructor() { print("P1::P1"); }
    instanceX() {
      return 1;
    }
    static staticX() {
      return 2;
    }
  }
  class P2 {
    constructor() { print("P2::P2"); }
    instanceX() {
      return 3;
    }
    static staticX() {
      return 4;
    }
  }
  class Child extends P1 {
    constructor() { super(); }
    instanceX() {
      return super.instanceX();
    }
    static staticX() {
      return super.staticX();
    }
  }

  let inst = new Child();
// CHECK-NEXT: P1::P1
  print(inst.instanceX());
// CHECK-NEXT: 1
  print(Child.staticX());
// CHECK-NEXT: 2

  // Only instance method should be affected.
  Object.setPrototypeOf(Child.prototype, P2.prototype);
  print(inst.instanceX());
// CHECK-NEXT: 3
  print(Child.staticX());
// CHECK-NEXT: 2

  // Now static method should be affected.
  Object.setPrototypeOf(Child, P2);
  print(inst.instanceX());
// CHECK-NEXT: 3
  print(Child.staticX());
// CHECK-NEXT: 4
})();

// Super loads in constructor are allowed.
(function () {
  class P1 {
    constructor() { print("P1::P1"); }
    x() {
      print("P1::x");
    }
  }
  class P2 {
    constructor() { print("P2::P2"); }
    x() {
      super.x();
      print("P2::x");
    }
  }
  let inst2 = new P2();
// CHECK-NEXT: P2::P2
  try {
    inst2.x();
  } catch (e) {
    print("call undefined");
// CHECK-NEXT: call undefined
  }
  Object.setPrototypeOf(P2.prototype, P1.prototype);
  inst2.x();
// CHECK-NEXT: P1::x
// CHECK-NEXT: P2::x
})();

// Cannot load from super before direct super call.
(function () {
  class P1 {
    constructor() { print("P1::P1"); }
    get foo() { print("P1::foo getter"); }
  }
  class C1 extends P1 {
    constructor() {
      super();
      super.foo;
    }
  }
  new C1();
// CHECK-NEXT: P1::P1
// CHECK-NEXT: P1::foo getter
  class C2 extends P1 {
    constructor() {
      super.foo;
      super();
    }
  }
  try {
    let inst1 = new C2();
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
  }
})();

// Computed keys of static and nonstatic are computed sequentially together.
(function () {
  let x = 0;
  class P1 {
    constructor() {}
    ["m" + (++x)]() {}
    static ["m" + (++x)]() {}
    ["m" + (++x)]() {}
    static ["m" + (++x)]() {}
  }
  var instanceMethods = Object.getOwnPropertyNames(P1.prototype);
  var staticMethods = Object.getOwnPropertyNames(P1);
  print(instanceMethods.includes("m1"));
// CHECK-NEXT: true
  print(staticMethods.includes("m2"));
// CHECK-NEXT: true
  print(instanceMethods.includes("m3"));
// CHECK-NEXT: true
  print(staticMethods.includes("m4"));
// CHECK-NEXT: true
})();

// Basic (static) getter/setter.
(function () {
  class P1 {
    static get x() {
      print("P1::x getter");
    }
    static set x(val) {
      print("P1::x setter", val);
    }
    get y() {
      print("P1::y getter");
    }
    set y(val) {
      print("P1::y setter", val);
    }
  }
  P1.x;
// CHECK-NEXT: P1::x getter
  P1.x = 12;
// CHECK-NEXT: P1::x setter 12
  let inst = new P1();
  inst.y;
// CHECK-NEXT: P1::y getter
  inst.y = 12;
// CHECK-NEXT: P1::y setter 12
})();

// Evaluation order of superClass.prototype and computed keys.
(function () {
  function foo() {}
  var x = 0;
  let prx = new Proxy(foo, {
    // This is invoked when the class definition tries to load the .prototype of its super class.
    get(target, prop, receiver) {
      print("proxy getter x:", x);
// CHECK-NEXT: proxy getter x: 0
      return {};
    },
  });
  class P1 extends prx {
    ["field" + (++x)]() {}
  }
  print("final x:", x);
// CHECK-NEXT: final x: 1
})();
