/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print('main');
// CHECK-LABEL: main

function simple(x, y) {
  ((z) => {
    print(this, arguments[0], arguments[1], z);
  })(3);
}
simple(1, 2);
// CHECK-NEXT: [object global] 1 2 3
simple.call(100, 1, 2);
// CHECK-NEXT: 100 1 2 3

function noArgs(x, y) {
  ((z) => {
    print(z);
  })(3);
}
noArgs(1, 2);
// CHECK-NEXT: 3

function nested(x, y) {
  ((z) => {
    (() => {
      print(this, arguments[0], arguments[1], z);
    })();
  })(3);
}
nested(1, 2);
// CHECK-NEXT: [object global] 1 2 3
nested.call(100, 1, 2);
// CHECK-NEXT: 100 1 2 3
