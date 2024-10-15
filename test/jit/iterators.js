/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo(x) {
  // NOTE(T57177012): This works without try/catch because we don't close the
  // iterator during array destructuring abrupt completions,
  // so there's no try/catch.
  // If that's fixed before the JIT supports try/catch,
  // this test will have to be updated.
  var [a, b, c] = x;
  print(a, b, c);
}

foo([1, 2, 3]);
// CHECK: 1 2 3

var it = {
  [Symbol.iterator]() {
    var i = 1;
    return {
      next() {
        if (i === 4) return {done: true};
        else return {done: false, value: i++};
      },
    };
  },
};

foo(it);
// CHECK-NEXT: 1 2 3
