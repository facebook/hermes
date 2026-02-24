/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-block-scoping -Xenable-tdz %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xes6-block-scoping %s | %FileCheck --match-full-lines %s

var arr = [];
(function () {
  arr.push("Loop let no-capture");
  for(let i = 0; i < 5; ++i) {
    arr.push(i);
  }

  arr.push("Loop let capture");
  for(let i = 0; i < 5; ++i) {
    arr.push(()=>i);
  }

  arr.push("Loop var capture");
  for(var i = 0; i < 5; ++i) {
    arr.push(()=>i);
  }

  arr.push("Loop inner let capture");
  for(var i = 0; i < 5; ++i) {
    let k = i;
    arr.push(()=>k + i);
  }

  arr.push("Finally loop let capture");
  try { return; } finally {
    for (let i = 0; i < 5; ++i) {
      arr.push(() => i);
    }
  }
})();

for(f of arr) {
  print(typeof f === "function" ? f() : f);
}

// CHECK: Loop let no-capture
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: Loop let capture
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: Loop var capture
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: 5
// CHECK-NEXT: Loop inner let capture
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: Finally loop let capture
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
