/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

class A {
    y = 7; // Field init for y.
    z: number;
    constructor() {
        // y is initialized before first statement of ctor without super.
        this.z = this.y + 1;
    }
}
print((new A()).y);
// CHECK: 7
print((new A()).z);
// CHECK: 8

// Fields are initialized in order.
class B {
    x = 76;
    y = this.x + 1;
    constructor() {}
}
print((new B()).y);
// CHECK: 77

// Implicit constructor is called, and does field inits.
class C0 {
    x = 775;
    m(): number {
        return this.x + 1;
    }
}
print((new C0()).x);
// CHECK: 775

// In a class with a superclass, a method invocation via super
// works and gets the right value; the super() constructor is
// executed before the field inits of the subclass.
class C1 extends C0 {
    y = super.m() + 1;
    constructor() {
        super();
    }
}
print((new C1()).y);
// CHECK: 777

// Field initializers may reference variables in their lexical scope.
function f(i: number): number {
    class A {
        x = i * 1000; // Gets the value of i from the invocation.
        constructor() {}
    }
    var a = new A();
    return a.x;
}
print(f(7));
// CHECK: 7000
