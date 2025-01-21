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

// Test that the receiver is set up correctly for reads.
(function () {
  var parent = {
    x: 10,
    get prop1() {
      print(this.x);
    }
  }
  var child = {
    x: 20,
    foo() {
      super.prop1;
    }
  }
  Object.setPrototypeOf(child, parent);
  child.foo();
//CHECK-NEXT: 20
})();

// Test that the receiver is set up correctly for writes.
(function () {
  var parent = {
    x: 30,
    set prop1(value) {
      print(this.x);
    }
  }
  var child = {
    x: 40,
    foo() {
      super.prop1 = "value";
    }
  }
  Object.setPrototypeOf(child, parent);
  child.foo();
//CHECK-NEXT: 40
})();

// Test that super writes throw under correct conditions.

// Should not throw
(function () {
  var parent = {}
  Object.defineProperty(parent, 'prop1', {
    value: 50,
    writable: false
  });
  var child = {
    foo() {
      super.prop1 = "value";
    }
  };
  Object.setPrototypeOf(child, parent);
  // This doesn't throw.
  child.foo();
  print(child.prop1);
//CHECK-NEXT: 50
  print(parent.prop1);
//CHECK-NEXT: 50
})();

// Should throw
(function () {
  "use strict";
  var parent = {}
  Object.defineProperty(parent, 'prop1', {
    value: 50,
    writable: false
  });
  var child = {
    foo() {
      super.prop1 = "value";
    }
  };
  Object.setPrototypeOf(child, parent);
  try {
    child.foo();
    print("Fail");
  } catch (e) {
    print("Pass");
  }
//CHECK-NEXT: Pass
})();

// super throws on null prototype.
(function () {
  function key() {
    return "x";
  }
  var obj = {
    reads() {
      try {
        super.x;
        print("read fail");
      } catch (err) {
        print("read threw", err.constructor.name);
      }
//CHECK-NEXT: read threw TypeError
      try {
        super[key()];
        print("read fail");
      } catch (err) {
        print("read threw", err.constructor.name);
      }
//CHECK-NEXT: read threw TypeError
    },
    writes() {
      try {
        super.x = 42;
        print("write fail");
      } catch (err) {
        print("write threw", err.constructor.name);
      }
//CHECK-NEXT: write threw TypeError
      try {
        super[key()] = 42;
        print("write fail");
      } catch (err) {
        print("write threw", err.constructor.name);
      }
//CHECK-NEXT: write threw TypeError
    }
  };
  Object.setPrototypeOf(obj, null);
  obj.reads();
  obj.writes();
})();
