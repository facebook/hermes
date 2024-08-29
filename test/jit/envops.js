/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xforce-jit -Xdump-jitcode %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

let x = 10;
let y = closure;

function closure(z) {
  print(x, z, y);
}

closure(20);
// CHECK: JIT successfully compiled FunctionID 1, 'closure'
// CHECK-NEXT: 10 20 function closure({{.*}}
