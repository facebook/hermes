// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

print('global');
// CHECK-LABEL: global
// isNaN()
print(isNaN(5));
// CHECK-NEXT: false

// isFinite()
print(isFinite(NaN));
// CHECK-NEXT: false

// parseInt()
var R_digit = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"];
for (var i = 2; i <= 36; i++) {
  print(parseInt(R_digit[i - 2] + "$", i));
}
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 9
// CHECK-NEXT: 10
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 16
// CHECK-NEXT: 17
// CHECK-NEXT: 18
// CHECK-NEXT: 19
// CHECK-NEXT: 20
// CHECK-NEXT: 21
// CHECK-NEXT: 22
// CHECK-NEXT: 23
// CHECK-NEXT: 24
// CHECK-NEXT: 25
// CHECK-NEXT: 26
// CHECK-NEXT: 27
// CHECK-NEXT: 28
// CHECK-NEXT: 29
// CHECK-NEXT: 30
// CHECK-NEXT: 31
// CHECK-NEXT: 32
// CHECK-NEXT: 33
// CHECK-NEXT: 34
// CHECK-NEXT: 35

// parseFloat()
print(parseFloat("-5.5"));
// CHECK-NEXT: -5.5

try {
  new isNaN();
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError

