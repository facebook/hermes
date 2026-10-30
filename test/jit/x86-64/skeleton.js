/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The first functions the x86-64 backend compiles natively: prologue,
// epilogue, the FR register file, and the LoadConst*/LoadParam/Mov/Ret
// emitters. Everything else still declines and runs in the interpreter, so
// each function's compile status is checked alongside its output.

function ret42() {
  return 42;
}
function identity(a) {
  return a;
}
function shuffle(a, b) {
  var t = a;
  a = b;
  b = t;
  return b;
}

print(ret42());
// CHECK: JIT successfully compiled FunctionID 1, 'ret42'
// CHECK: 42
print(identity("xy"));
// CHECK: JIT successfully compiled FunctionID 2, 'identity'
// CHECK: xy
print(shuffle(1, 2));
// CHECK: JIT successfully compiled FunctionID 3, 'shuffle'
// CHECK: 1
