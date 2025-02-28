/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-memory-limit=0 -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo(x) {
  return 0;
}

for (var i = 0; i < 100; ++i)
  foo();

// CHECK-NOT: JIT successfully compiled FunctionID {{.*}}, 'foo'
