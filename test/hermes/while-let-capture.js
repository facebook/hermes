/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-block-scoping %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Xes6-block-scoping -exec %s | %FileCheck --match-full-lines %s

(function (){
  let arr = [];
  let i = 0;
  while (i < 10) {
    let j = i;
    arr.push(()=>j);
    i++;
  }

  for(f of arr) {
    print(f());
  }
})();

// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
