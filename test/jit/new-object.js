/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xforce-jit -Xdump-jitcode %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function newObject() {
  return {};
}

function newObjectWithParent() {
  return {__proto__: Math};
}

function newObjectWithBuffer() {
  return {a: 1, b: 3};
}

print("newObject", newObject());
// CHECK: JIT successfully compiled FunctionID 1, 'newObject'
// CHECK-NEXT: newObject [object Object]
print("newObjectWithParent", Object.getPrototypeOf(newObjectWithParent()));
// CHECK: JIT successfully compiled FunctionID 2, 'newObjectWithParent'
// CHECK-NEXT: newObjectWithParent [object Math]
print("newObjectWithBuffer", newObjectWithBuffer().a);
// CHECK: JIT successfully compiled FunctionID 3, 'newObjectWithBuffer'
// CHECK-NEXT: newObjectWithBuffer 1
