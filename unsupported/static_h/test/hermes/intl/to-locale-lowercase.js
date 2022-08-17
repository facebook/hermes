/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get locale lowercase test");
// CHECK-LABEL: get locale lowercase test

print('TEST'.toLocaleLowerCase());
// CHECK-NEXT: test

print('İstanbul'.toLocaleLowerCase('en-US'));
// CHECK-NEXT: i̇stanbul

print('İstanbul'.toLocaleLowerCase('tr'));
// CHECK-NEXT: istanbul

print('CAFÉTÉRIA İĮ̀'.toLocaleLowerCase('en-NZ'));
// CHECK-NEXT: cafétéria i̇į̀

print('CAFÉTÉRIA İĮ̀'.toLocaleLowerCase('tr'));
// CHECK-NEXT: cafétérıa iį̀

print('CAFÉTÉRIA İĮ̀'.toLocaleLowerCase('lt'));
// CHECK-NEXT: cafétéria i̇į̇̀

print('\U0001f3eb,̆IA\u0307'.toLocaleLowerCase('cs-CZ'));
// CHECK-NEXT: u0001f3eb,̆iȧ

print(''.toLocaleLowerCase('en-US').length);
// CHECK-NEXT: 0
