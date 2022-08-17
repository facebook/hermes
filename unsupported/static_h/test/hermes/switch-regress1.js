/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Ensure that switch statements that are backwards branches work properly.

(function() {
  var j = 0;
  for (var i = 0; i < 4; i++) {
    ++j;
    if (j > 5) {
      return;
    }
    print(i);
    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        break;
    }
    i = 2;
  }
})();

// CHECK: 0
// CHECK: 3
// CHECK: 3
// CHECK: 3
// CHECK: 3
