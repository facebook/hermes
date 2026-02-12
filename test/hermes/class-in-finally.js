/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Test that classes work correctly when defined inside finally blocks.
// Finally blocks emit code twice (normal flow + exception handler),
// which requires proper caching of internal variables.

function testStaticProperty() {
    try {
        throw 1;
    } finally {
        class C {
            static g = 42;
        }
        print(C.g);
// CHECK: 42
    }
}
try { testStaticProperty(); } catch (e) {}
print("testStaticProperty passed");
// CHECK-NEXT: testStaticProperty passed

function testInstanceProperty() {
    try {
        throw 1;
    } finally {
        class C {
            f = 52;
        }
        print(new C().f);
// CHECK-NEXT: 52
    }
}
try { testInstanceProperty(); } catch (e) {}
print("testInstanceProperty passed");
// CHECK-NEXT: testInstanceProperty passed

function testComputedProperty() {
    var key = "x";
    try {
        throw 1;
    } finally {
        class C {
            static [key] = 42;
            [key] = 52;
        }
        print(C[key]);
// CHECK-NEXT: 42
        print(new C()[key]);
// CHECK-NEXT: 52
    }
}
try { testComputedProperty(); } catch (e) {}
print("testComputedProperty passed");
// CHECK-NEXT: testComputedProperty passed

function testPrivateMethod() {
    try {
        throw 1;
    } finally {
        class C {
            #m() { return 62; }
            call() { return this.#m(); }
        }
        print(new C().call());
// CHECK-NEXT: 62
    }
}
try { testPrivateMethod(); } catch (e) {}
print("testPrivateMethod passed");
// CHECK-NEXT: testPrivateMethod passed

function testStaticPrivateMethod() {
    try {
        throw 1;
    } finally {
        class C {
            static #m() { return 72; }
            static call() { return C.#m(); }
        }
        print(C.call());
// CHECK-NEXT: 72
    }
}
try { testStaticPrivateMethod(); } catch (e) {}
print("testStaticPrivateMethod passed");
// CHECK-NEXT: testStaticPrivateMethod passed

function testPrivateAccessor() {
    try {
        throw 1;
    } finally {
        class C {
            #val = 82;
            get #x() { return this.#val; }
            set #x(v) { this.#val = v; }
            call() { this.#x = this.#x + 10; return this.#x; }
        }
        print(new C().call());
// CHECK-NEXT: 92
    }
}
try { testPrivateAccessor(); } catch (e) {}
print("testPrivateAccessor passed");
// CHECK-NEXT: testPrivateAccessor passed
