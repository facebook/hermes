/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("get locale uppercase test");
// CHECK-LABEL: get locale uppercase test

print('test'.toLocaleUpperCase());
// CHECK-NEXT: TEST

print('istanbul'.toLocaleUpperCase('en-US'));
// CHECK-NEXT: ISTANBUL

print('istanbul'.toLocaleUpperCase('TR'));
// CHECK-NEXT: İSTANBUL

print('STRAßE'. toLocaleUpperCase('DE-de'));
// CHECK-NEXT: STRASSE

print('cafétéria iį̀'.toLocaleUpperCase('en-NZ'));
// CHECK-NEXT: CAFÉTÉRIA IĮ̀

print('cafétéria iį̀'.toLocaleUpperCase('tr'));
// CHECK-NEXT: CAFÉTÉRİA İĮ̀

print(''.toLocaleUpperCase('en-US').length);
// CHECK-NEXT: 0
