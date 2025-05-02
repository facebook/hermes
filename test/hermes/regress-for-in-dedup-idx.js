/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var Test = function () {};
var ii = 0;
function opt() {
  print('ii:', ii); // 0 4294967294
  Test.prototype[ii] = 0;
  Test.prototype[ii + 2] = 2;
}
opt();
var test1 = new Test(); // 0 2

for (let m in test1) {
  if (m == 2) {
    ii = 4294967294; //0xFFFFFFFE
    opt();
  }
  print('m', m);
}

for (let n in test1) {
  print('n:', n);
}

// CHECK-LABEL: ii: 0
// CHECK-NEXT: m 0
// CHECK-NEXT: ii: 4294967294
// CHECK-NEXT: m 2
// CHECK-NEXT: n: 0
// CHECK-NEXT: n: 2
// CHECK-NEXT: n: 4294967294
// CHECK-NEXT: n: 4294967296
