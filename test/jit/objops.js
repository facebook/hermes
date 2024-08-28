/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xforce-jit -Xdump-jitcode %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// GetGlobalObject
// PutByIdLoose
// TryGetById
function foo() {
    a = 10;
    b = 20;
    return a + b;
}

// PutByIdLoose
// PutByValLoose
// GetById
// GetByVal
function bar(o, p) {
    o.a = 1;
    o.b = 2;
    o[p] = 100;
    return o.a + o.b + o[p];
}

function isIn(a, prop) {
    return prop in a;
}

print("foo", foo());
// CHECK: JIT successfully compiled FunctionID 1, 'foo'
// CHECK-NEXT: foo 30
print("bar", bar({}, "prop"));
// CHECK: JIT successfully compiled FunctionID 2, 'bar'
// CHECK-NEXT: bar 103
print("isIn", isIn({}, "prop"));
// CHECK: JIT successfully compiled FunctionID 3, 'isIn'
// CHECK-NEXT: isIn false
print("isIn", isIn({prop: 1}, "prop"));
// CHECK-NEXT: isIn true
