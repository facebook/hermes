/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// Test for a bug where lazy compilation did not deduplicate functions that were
// emitted multiple times, resulting in a crash when trying to compile it the
// second time.
for (v of [true, false]) {
  (function (){
    try {
      // On the first iteration, execute the finally through the return path.
      if (v) return;
      // On the second iteration, fall through into the finally.
      print("apple");
    } finally {
      (function () {
        print("banana");
      })();
    }
  })();
}

// CHECK-LABEL: banana
// CHECK-NEXT: apple
// CHECK-NEXT: banana
