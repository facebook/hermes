/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -fstatic-builtins -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

// CallBuiltin's "this" is never populated by bytecode, so the runtime helper
// must initialize it to undefined. Every call in a function shares the same
// outgoing ThisArg slot, so a preceding ordinary call leaves its receiver
// there; if the builtin call does not overwrite it, Array.from() sees that
// stale receiver as its "this" and uses it as the constructor C.

function Poison(n) {
  this.iAmPoison = true;
  this.length = n;
}
Poison.probe = function () {
  return 1;
};

function test(arr) {
  // Ordinary call whose "this" is Poison, poisoning the shared slot.
  Poison.probe();
  // CallBuiltin: must not observe Poison as its "this".
  return Array.from(arr);
}

var r = test([10, 20, 30]);

print(Array.isArray(r));
// CHECK: true
print(r.constructor === Array);
// CHECK-NEXT: true
print(r.iAmPoison);
// CHECK-NEXT: undefined
print(r[0], r[1], r[2]);
// CHECK-NEXT: 10 20 30
