/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xenable-tdz -fno-inline -Xforce-jit -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function outer(flag) {
  var f = () => x;
  if (flag === 1) f();
  let x;
  if (flag === 2) f();
  return 'done';
}


try { outer(1); } catch(e) { print(e.name); }
// CHECK: JIT successfully compiled FunctionID 1, 'outer'
// CHECK: JIT successfully compiled FunctionID 2, 'f'
// CHECK: ReferenceError
print(outer(2));
// CHECK-NEXT: done
