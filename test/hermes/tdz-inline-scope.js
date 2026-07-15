/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xenable-tdz -Xes6-block-scoping -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xenable-tdz -Xes6-block-scoping -O %s | %FileCheck --match-full-lines %s

// The arrow assigned to `x` is created while evaluating the iterable, where the
// loop's `y` is still in TDZ, so it captures the head-scope `y` instance, which
// is never initialized. The per-iteration `y` (the loop binding) is a different
// instance of the same Variable. When `x()` is inlined into the generator,
// TDZDedup must not drop x's TDZ check just because the initialized
// per-iteration `y` was already checked: calling `x` must still throw.
function* test() {
  let x;
  for (let y of [x = () => y]) {
    try {
      x();
      print("wrong");
    } catch (e) {
      print(e);
    }
  }
}

test().next();

// CHECK: ReferenceError: accessing an uninitialized variable
