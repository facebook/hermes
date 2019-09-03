// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

// Math constructor is not callable. Serialize directly.
serializeVM(function() {
  // Test some Math functions.
  print('Math');
  // CHECK-LABEL: Math
  print(Math.toString());
  // CHECK-NEXT: [object Math]
  print(Math.E);
  // CHECK-NEXT: 2.718281828459045
  print(Math.LN10);
  // CHECK-NEXT: 2.302585092994046
  print(Math.LN2);
  // CHECK-NEXT: 0.6931471805599453
  print(Math.LOG2E);
  // CHECK-NEXT: 1.4426950408889634
  print(Math.LOG10E);
  // CHECK-NEXT: 0.4342944819032518
  print(Math.PI);
  // CHECK-NEXT: 3.141592653589793
  print(Math.SQRT1_2);
  // CHECK-NEXT: 0.7071067811865476
  print(Math.SQRT2);
  // CHECK-NEXT: 1.4142135623730951
  // Ensure they're not writable
  Math.PI = 4;
  print(Math.PI);
  // CHECK-NEXT: 3.141592653589793
})
