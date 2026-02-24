/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -O0 %s | %FileCheck --match-full-lines %s

// Checking scenarios where the string is added as an identifier
// and a non-identifier in succession.

print('a', globalThis.b, 'c');
// CHECK-LABEL: a undefined c

(function() {
  // literal to identifier
  print(globalThis.a);
// CHECK-NEXT: undefined

  // identifier to identifier
  print(globalThis.b);
// CHECK-NEXT: undefined

  // literal to literal
  print(globalThis.c);
// CHECK-NEXT: undefined

  // literal to identifier
  try { print(global.Math); } catch (e) { print(e.name); }
// CHECK-NEXT: ReferenceError

  print('d');
// CHECK-NEXT: d
  print(globalThis.d);
// CHECK-NEXT: undefined
})();
