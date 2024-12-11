/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

class A {
  baseInstMethod() {
    return "baseInstMethod";
  }
  static baseStaticMethod() {
    return "baseStaticMethod";
  }
}

class B extends A {
  derivedInstMethod() {
    debugger;
  }
  static derivedStaticMethod() {
    debugger;
  }
}

new B().derivedInstMethod();
B.derivedStaticMethod();

// CHECK: Break on 'debugger' statement in derivedInstMethod: {{.*}}:22:5
// CHECK-NEXT: baseInstMethod
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in derivedStaticMethod: {{.*}}:25:5
// CHECK-NEXT: baseStaticMethod
// CHECK-NEXT: Continuing execution


class C {
  constructor(p1) {
    print("C::C");
    this.p1 = p1;
  }
}

// `this` in a derived constructor
class D extends C {
  constructor(p1, p2) {
    debugger;
    super(p1);
    this.p2 = p2;
    debugger;
  }
}
new D(10, 20);

// `this` in a derived constructor, in an arrow function.
class E extends C {
  constructor(p1, p2) {
    let arr1 = () => { 
      let arr2 = () => { debugger; }
      arr2();
    }
    arr1();
    super(p1);
    this.p2 = p2;
    arr1();
  }
}
new E(30, 40);

// Invoking `super()`.
class F extends C {
  constructor(p1, p2) {
    debugger;
    super(p1);
  }
}
try {
  new F(50, 60);
} catch (e) {
  print(e);
}

// CHECK-NEXT: Break on 'debugger' statement in D: {{.*}}:50:5
// CHECK-NEXT: Exception: ReferenceError: accessing an uninitialized variable
// CHECK: Continuing execution
// CHECK-NEXT: C::C
// CHECK-NEXT: Break on 'debugger' statement in D: {{.*}}:53:5
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in arr2: {{.*}}:62:26
// CHECK-NEXT: Exception: ReferenceError: accessing an uninitialized variable
// CHECK: Continuing execution
// CHECK-NEXT: C::C
// CHECK-NEXT: Break on 'debugger' statement in arr2: {{.*}}:62:26
// CHECK-NEXT: 30
// CHECK-NEXT: 40
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in F: {{.*}}:76:5
// CHECK-NEXT: C::C
// CHECK-NEXT: { p1: 50 }
// CHECK-NEXT: C::C
// CHECK-NEXT: Exception: ReferenceError: Cannot call super constructor twice
// CHECK: Continuing execution
// CHECK-NEXT: C::C
// CHECK-NEXT: ReferenceError: Cannot call super constructor twice
