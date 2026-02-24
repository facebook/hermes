/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines

// Test that typed classes work correctly when defined inside finally blocks.

function testTypedClassField() {
    try {
        throw 1;
    } finally {
        class C {
            x: number = 1;
        }
        new C();
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
        new C().getX();
    }
}

try { testTypedClassField(); } catch (e) {}
print("testTypedClassField passed");
// CHECK: testTypedClassField passed

try { testTypedClassMethod(); } catch (e) {}
print("testTypedClassMethod passed");
// CHECK-NEXT: testTypedClassMethod passed
