/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xforce-jit -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

var a = 10;
print(a);
// CHECK: JIT successfully compiled FunctionID 0, 'global'
// CHECK-NEXT: 10
