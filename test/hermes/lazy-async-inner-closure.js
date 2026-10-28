/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print("lazy async inner closure");
// CHECK-LABEL: lazy async inner closure

function makeAsync(base) {
  var count = 0;
  async function f(delta) {
    count += delta;
    return await Promise.resolve(base + count);
  }
  return f;
}

var f = makeAsync(10);
f(1).then(function(value) {
  print(value);
});
f(2).then(function(value) {
  print(value);
});

function makeArrow(base) {
  return {
    value: 5,
    make: function(extra) {
      return async (delta) => {
        return await Promise.resolve(this.value + arguments[0] + base + delta);
      };
    },
  }.make(7);
}

var arrow = makeArrow(100);
arrow(3).then(function(value) {
  print(value);
});
arrow(4).then(function(value) {
  print(value);
});

// CHECK-NEXT: 11
// CHECK-NEXT: 13
// CHECK-NEXT: 115
// CHECK-NEXT: 116
