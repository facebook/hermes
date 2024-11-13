/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s -Xes6-block-scoping | %FileCheck --match-full-lines %s
// RUN: %hermes -Xes6-block-scoping -lazy %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Xes6-block-scoping -exec %s | %FileCheck --match-full-lines %s

let arr = (function (){
  let arr = [];
  try {
    while (true) {
      let x = 42;
      // Make sure there is a capture to force a scope.
      arr.push(()=>x);
      return arr;
    }
  } finally {
    // Test that emitting this finally block at the location of the return above
    // (which is a different scope in the IR) works correctly.
    while (arr.length < 10) {
      let x = arr.length;
      arr.push(()=>x);
    }
  }
})();

for (v of arr){
  print(v());
}

// CHECK: 42
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
