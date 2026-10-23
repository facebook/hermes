/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict %s > %t.js
// RUN: cat %s >> %t.js
// RUN: %hermes -non-strict -lazy=0 -Xjit=force -Xjit-crash-on-error %t.js | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The prepended filler() occupies the first 5000 shape table entries. Indices
// above 4095 don't fit in the immediate offset of an arm64 load, so the JIT
// must materialize the offset in a register.

function f(p) {
  return [{__proto__: p}, {q0: 1, q1: 2}];
}

// Running this file alone generates filler(). Once prepended, filler is
// defined and the test runs instead.
if (typeof filler !== 'function') {
  var parts = [];
  for (var i = 0; i < 5000; ++i) parts.push('{p' + i + ':1},');
  print('function filler(){return[' + parts.join('') + '];}');
} else {
  var proto = {x: 10};
  for (var i = 0; i < 3; ++i) {
    var r = f(proto);
    print(Object.getPrototypeOf(r[0]) === proto, r[0].x, r[1].q0, r[1].q1);
  }
}
// CHECK: true 10 1 2
// CHECK-NEXT: true 10 1 2
// CHECK-NEXT: true 10 1 2
