/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// Regression test for NewObjectWithParent clobbering its parent operand.
//
// NewObjectWithParent used to compute the new object's
// (possibly compressed) parent pointer *in place*, overwriting the register
// holding the parent operand, on the assumption that the operand was dead.
//
// Here the parent is the `null` literal, which the register allocator pins to
// a global (callee-saved) register that stays live well past the object
// creation -- it is reused below as the right-hand side of `el == null`.
// Clobbering that register left the comparison reading garbage (zero) instead
// of `null`, so `null == null` produced `false`.

(function () {
  var mixed: (number | void | null)[] = [1, null];
  for (var i: number = 0; i < mixed.length; ++i) {
    var el = mixed[i];
    print(i, el == null);
  }
})();
// CHECK: 0 false
// CHECK-NEXT: 1 true
