/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xjit=force -Xdump-jitcode=2 -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function loadThisNS() {
  return this;
}

print("loadThisNS", loadThisNS());
// CHECK: JIT successfully compiled FunctionID 1, 'loadThisNS'
// CHECK-NEXT: loadThisNS [object global]
