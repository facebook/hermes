/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

// Test that typed classes work correctly when defined inside finally blocks.

// IRGen compiles a finally block twice, once for the normal path and once for
// the exceptional one. Each function below exercises one of the two copies, and
// prints the values so that a copy wired to the wrong Variable is visible
// rather than swallowed by the caller's catch.

function testTypedClassField() {
    try {
        throw 1;
    } finally {
        class C {
            x: number = 1;
            static y: number = 2;
        }
        print(C.y, new C().x);
    }
}

function testTypedClassFieldNormal(): void {
    try {
        return;
    } finally {
        class C {
            x: number = 1;
            static y: number = 2;
        }
        print(C.y, new C().x);
    }
}

function testTypedClassMethod() {
    try {
        throw 1;
    } finally {
        class C {
            x: number = 1;
            getX(): number { return this.x; }
        }
        print(new C().getX());
    }
}

function testTypedClassMethodNormal(): void {
    try {
        return;
    } finally {
        class C {
            x: number = 1;
            getX(): number { return this.x; }
        }
        print(new C().getX());
    }
}

try { testTypedClassField(); } catch (e) {}
// CHECK: 2 1
testTypedClassFieldNormal();
// CHECK-NEXT: 2 1
print("testTypedClassField passed");
// CHECK-NEXT: testTypedClassField passed

try { testTypedClassMethod(); } catch (e) {}
// CHECK-NEXT: 1
testTypedClassMethodNormal();
// CHECK-NEXT: 1
print("testTypedClassMethod passed");
// CHECK-NEXT: testTypedClassMethod passed
