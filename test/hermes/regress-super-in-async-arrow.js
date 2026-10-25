/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// The enclosing class context must be propagated through the outer async and
// generator functions which wrap the body of an async arrow function, so that
// super() and 'this' work correctly inside it.

print("super in async arrow");
// CHECK-LABEL: super in async arrow

class Base {
  constructor(x) {
    this.x = x;
  }
}

// super() in a default parameter initializer of an async arrow.
var d1 = new (class extends Base {
  constructor() {
    (async (a = super(1), b) => {})();
  }
})();
print('param', d1.x);
// CHECK-NEXT: param 1

// super() in the body of an async arrow.
var d2 = new (class extends Base {
  constructor() {
    (async () => {
      super(2);
    })();
  }
})();
print('body', d2.x);
// CHECK-NEXT: body 2

// super() in an async arrow nested inside a regular arrow.
var d3 = new (class extends Base {
  constructor() {
    (() => {
      (async () => {
        super(3);
      })();
    })();
  }
})();
print('nested', d3.x);
// CHECK-NEXT: nested 3

// 'this' in an async arrow of a base constructor. The body of an async function
// runs synchronously up to the first await, so this prints in source order.
class B4 {
  constructor() {
    this.x = 4;
    (async () => {
      print('base this', this.x);
    })();
  }
}
new B4();
// CHECK-NEXT: base this 4

// Private fields and methods are reachable from an async arrow in a
// constructor.
class P5 {
  #v = 5;
  #m() {
    return this.#v;
  }
  constructor() {
    (async () => {
      print('private', this.#m(), #v in this);
    })();
  }
}
new P5();
// CHECK-NEXT: private 5 true

// super property access from a generator method.
class S6 {
  foo() {
    return 'S6.foo';
  }
}
class S7 extends S6 {
  foo() {
    return 'S7.foo';
  }
  *gen() {
    yield super.foo();
  }
  async am() {
    return super.foo();
  }
}
var s7 = new S7();
print('generator super', s7.gen().next().value);
// CHECK-NEXT: generator super S6.foo

// 'this' in an async arrow of a derived constructor is still guarded: reading
// it before super() must throw a ReferenceError. The rejection handler runs as
// a microtask, i.e. after all the synchronous output above.
class D8 extends Base {
  constructor() {
    var f = async () => this.x;
    f().then(
      function () {
        print('derived this: resolved');
      },
      function (e) {
        print('derived this:', e.name);
      },
    );
    super(8);
  }
}
new D8();

// super property access from an async method, also resolved as a microtask.
s7.am().then(function (v) {
  print('async method super', v);
});

// CHECK-NEXT: derived this: ReferenceError
// CHECK-NEXT: async method super S6.foo
