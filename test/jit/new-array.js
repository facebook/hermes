/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xforce-jit -Xdump-jitcode %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function newArray() {
  return [()=>{}, ()=>{}];
}

function newArrayWithBuffer() {
  return [0, 1, 2];
}

print("newArray", newArray().length);
// CHECK: JIT successfully compiled FunctionID 1, 'newArray'
// CHECK-NEXT: newArray 2
print("newArrayWithBuffer", newArrayWithBuffer()[1]);
// CHECK: JIT successfully compiled FunctionID 2, 'newArrayWithBuffer'
// CHECK-NEXT: newArrayWithBuffer 1
