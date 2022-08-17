/**
 * Portions Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* This code is adapted from Test262, which has the following copyright notice:
 *
 * The << Software identified by reference to the Ecma Standard* ("Software)">>  is protected by copyright and is being
 * made available under the  "BSD License", included below. This Software may be subject to third party rights (rights
 * from parties other than Ecma International), including patent rights, and no licenses under such third party rights
 * are granted under this license even if the third party concerned is a member of Ecma International.  SEE THE ECMA
 * CODE OF CONDUCT IN PATENT MATTERS AVAILABLE AT http://www.ecma-international.org/memento/codeofconduct.htm FOR
 *INFORMATION REGARDING THE LICENSING OF PATENT CLAIMS THAT ARE REQUIRED TO IMPLEMENT ECMA INTERNATIONAL STANDARDS*.
 *
 * Copyright (C) 2012-2013 Ecma International
 * All rights reserved.
*/

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl
// These tests are a working subset of test262/test/intl402/String/prototype/toLocaleLowerCase

print("get locale lowercase test for test262");
// CHECK-LABEL: get locale lowercase test for test262

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
