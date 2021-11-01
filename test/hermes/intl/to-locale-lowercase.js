/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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

print('\u0130'.toLocaleLowerCase('und') === '\u0069\u0307');
// CHECK-NEXT: true

print('\u0130'.toLocaleLowerCase('az'));
// CHECK-NEXT: i

print('I\u0307'.toLocaleLowerCase('az'));
// CHECK-NEXT: i

print('IA\u0307'.toLocaleLowerCase('az') === '\u0131a\u0307');
// CHECK-NEXT: true

print('I\u0300\u0307'.toLocaleLowerCase('az') === '\u0131\u0300\u0307');
// CHECK-NEXT: true

print('I\uD834\uDD85\u0307'.toLocaleLowerCase('az') === '\u0131\uD834\uDD85\u0307');
// CHECK-NEXT: true

print('I'.toLocaleLowerCase('az') === '\u0131');
// CHECK-NEXT: true

print('i'.toLocaleLowerCase('az') === 'i');
// CHECK-NEXT: true

print('\u0131'.toLocaleLowerCase('az') === '\u0131');
// CHECK-NEXT: true

print('\u0130'.toLocaleLowerCase('tr'));
// CHECK-NEXT: i

print('I\u0307'.toLocaleLowerCase('tr'));
// CHECK-NEXT: i

print('IA\u0307'.toLocaleLowerCase('tr') === '\u0131a\u0307');
// CHECK-NEXT: true

print('I\u0300\u0307'.toLocaleLowerCase('tr') === '\u0131\u0300\u0307');
// CHECK-NEXT: true

print('I\uD834\uDD85\u0307'.toLocaleLowerCase('tr') === '\u0131\uD834\uDD85\u0307');
// CHECK-NEXT: true

print('I'.toLocaleLowerCase('tr') === '\u0131');
// CHECK-NEXT: true

print('i'.toLocaleLowerCase('tr') === 'i');
// CHECK-NEXT: true

print('\u0131'.toLocaleLowerCase('tr') === '\u0131');
// CHECK-NEXT: true

print('I\u0300'.toLocaleLowerCase('lt') === 'i\u0307\u0300');
// CHECK-NEXT: true

print('J\u0300'.toLocaleLowerCase('lt') === 'j\u0307\u0300');
// CHECK-NEXT: true

print('\u012E\u0300'.toLocaleLowerCase('lt') === '\u012F\u0307\u0300');
// CHECK-NEXT: true

print('I\u0325\u0300'.toLocaleLowerCase('lt') === 'i\u0307\u0325\u0300');
// CHECK-NEXT: true

print('J\u0325\u0300'.toLocaleLowerCase('lt') === 'j\u0307\u0325\u0300');
// CHECK-NEXT: true

print('\u012E\u0325\u0300'.toLocaleLowerCase('lt') === '\u012F\u0307\u0325\u0300');
// CHECK-NEXT: true

print('IA\u0300'.toLocaleLowerCase('lt') === 'ia\u0300');
// CHECK-NEXT: true

print('JA\u0300'.toLocaleLowerCase('lt') === 'ja\u0300');
// CHECK-NEXT: true

print('\u012EA\u0300'.toLocaleLowerCase('lt') === '\u012Fa\u0300');
// CHECK-NEXT: true

print('IA\uD834\uDD85'.toLocaleLowerCase('lt') === 'ia\uD834\uDD85');
// CHECK-NEXT: true

print('JA\uD834\uDD85'.toLocaleLowerCase('lt') === 'ja\uD834\uDD85');
// CHECK-NEXT: true

print('\u012EA\uD834\uDD85'.toLocaleLowerCase('lt') === '\u012Fa\uD834\uDD85');
// CHECK-NEXT: true

print('\u00CC'.toLocaleLowerCase('lt') === '\u0069\u0307\u0300');
// CHECK-NEXT: true

print('\u00CD'.toLocaleLowerCase('lt') === '\u0069\u0307\u0301');
// CHECK-NEXT: true

print('\u0128'.toLocaleLowerCase('lt') === '\u0069\u0307\u0303');
// CHECK-NEXT: true
