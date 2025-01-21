/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print('Classes');
// CHECK-LABEL: Classes

class A {
  constructor(a) {
    this.a = a;
  }
}

// Happy, easy case.
(function () {
  class B extends A {
    constructor(a, b) {
      super(a);
      this.b = b;
    }
  }
  let instance = new B(1, 2);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
})();

// Arrow functions call super too.
(function () {
  class B extends A {
    constructor(a, b) {
      let arrow1 = () => {
        let arrow2 = () => {
          super(a);
        };
        arrow2();
      };
      arrow1();
      this.b = b;
    }
  }
  let instance = new B(1, 2);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
})();

// Arrow functions can set properties on `this`.
(function () {
  class B extends A {
    constructor(a, b) {
      let arrow1 = () => {
        super(a);
        this.b = b;
      };
      arrow1();
    }
  }
  let instance = new B(1, 2);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
})();

// Arrow functions cannot use uninitialized `this`.
(function () {
  class B extends A {
    constructor(a, b) {
      let arrow1 = () => {
        this.b = b;
      };
      arrow1();
    }
  }
  try {
  let instance = new B(1, 2);
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
  }
})();

// Throw on multiple super calls.
(function () {
  class B extends A {
    constructor() {
      super(1);
      super(1);
    }
  }
  try {
    new B();
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: Cannot call super constructor twice
  }
})();

// Allow duplicate super calls to run, but throw after second completion.
(function () {
  class B {
    constructor(x, cb) {
      print("Base called", x);
      cb();
      print("Base returning", x);
    }
  }
  class C extends B {
    constructor() {
      let arrow = () => { super(2, () => { super(3, () => {}); }); }
      super(1, arrow);
    }
  }
  try {
    new C();
// CHECK-NEXT: Base called 1
// CHECK-NEXT: Base called 2
// CHECK-NEXT: Base called 3
// CHECK-NEXT: Base returning 3
// CHECK-NEXT: Base returning 2
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: Cannot call super constructor twice
  }
})();

// Don't need to call super when constructing new object.
(function () {
  class B extends A {
    constructor(b) {
      return {b: b};
    }
  }
  let instance = new B(2);
  print(instance.b);
// CHECK-NEXT: 2
})();

// Returning object discards this parameter.
(function () {
  class B extends A {
    constructor(a, b) {
      super(a);
      return {b: b};
    }
  }
  let instance = new B(1, 2);
  print(instance.a);
// CHECK-NEXT: undefined
  print(instance.b);
// CHECK-NEXT: 2
})();

// Not calling super is an error.
(function () {
  class B extends A {
    constructor() {}
  }
  try {
    new B();
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
  }
})();

// Returning undefined in subclass works.
(function () {
  class B extends A {
    constructor(a, b) {
      super(a);
      this.b = b;
      return undefined;
    }
  }
  let instance = new B(1, 2);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
})();

// Returning non-(object|undefined) in subclass throws.
(function () {
  class B extends A {
    constructor(a) {
      super(a);
      return 42;
    }
  }
  try {
    new B(1);
// CHECK-NEXT: TypeError: Derived constructors may only return an object or undefined.
  } catch (e) {
    print(e);
  }
  class C extends A {
    constructor(a) {
      super(a);
      return null;
    }
  }
  try {
    new C(1);
// CHECK-NEXT: TypeError: Derived constructors may only return an object or undefined.
  } catch (e) {
    print(e);
  }
})();

// Returning function in subclass is fine.
(function () {
  class B extends A {
    constructor() {
      return function foo() { return 15; };
    }
  }
  print(new B()());
// CHECK-NEXT: 15
})();

// Returning non-object in base class is fine.
(function () {
  class B {
    constructor(a) {
      this.a = a;
      return 42;
    }
  }
  class C extends B {
    constructor(a, b) {
      super(a);
      this.b = b;
    }
  }
  let instance = new C(1, 2);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
})();

// Returning `this` in a derived class constructor is checked.
(function () {
  class B extends A {
    constructor(a) {
      return this;
    }
  }
  try {
    let instance = new B(1, 2);
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
  }
  class C extends A {
    constructor(a) {
      return (1, this);
    }
  }
  try {
    let instance = new C(1, 2);
  } catch (e) {
    print(e);
// CHECK-NEXT: ReferenceError: accessing an uninitialized variable
  }
})();

// Test we can spread values into a super call.
(function () {
  class Base {
    constructor(a, b, c) {
      this.a = a;
      this.b = b;
      this.c = c;
    }
  }
  class C1 extends Base {
    constructor(a, b, c) {
      let arr = [a, b, c];
      super(...arr);
    }
  }
  class C2 extends Base {
    constructor() {
      super(...arguments);
    }
  }
  let instance = new C1(1, 2, 3);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
  print(instance.c);
// CHECK-NEXT: 3

  instance = new C2(1, 2, 3);
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
  print(instance.c);
// CHECK-NEXT: 3
})();

// Ensure new.target is correctly propagated through a super() with spread args.
(function () {
  class Base {
    constructor() {
      print("Base", new.target.name);
    }
  }
  class C1 extends Base {
    constructor() {
      print("C1", new.target.name);
      super(...arguments);
    }
  }
  new C1();
// CHECK-NEXT: C1 C1
// CHECK-NEXT: Base C1

  function foo() {}
  Reflect.construct(C1, [], foo);
// CHECK-NEXT: C1 foo
// CHECK-NEXT: Base foo
})();

// Test default constructor.
(function () {
  class Base {
    constructor(a, b, c) {
      print(new.target.name);
      this.a = a;
      this.b = b;
      this.c = c;
    }
  }
  class C1 extends Base {}
  let instance = new C1(1, 2, 3);
// CHECK-NEXT: C1
  print(instance.a);
// CHECK-NEXT: 1
  print(instance.b);
// CHECK-NEXT: 2
  print(instance.c);
// CHECK-NEXT: 3

  function foo() {}
  Reflect.construct(C1, [], foo);
// CHECK-NEXT: foo
})();

// Arguments to super are evaluated before a bad constructor invocation.
(function () {
  function foo() {
    print("foo called");
  }
  class C extends Object {
    constructor() {
      try {
        super(foo());
      } catch (e) {
        print("error thrown");
      }
      return {};
    }
  }
  Object.setPrototypeOf(C, parseInt);
  new C();
// CHECK-NEXT: foo called
// CHECK-NEXT: error thrown
})();
