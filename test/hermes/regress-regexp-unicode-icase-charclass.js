/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Regression test: /\w/iu must match characters whose Unicode canonical form
// is a word character (e.g. U+212A KELVIN SIGN folds to 'k').

// U+212A KELVIN SIGN should match \w only with /iu flags.
print(/\w/iu.test('\u212A'));
// CHECK: true
print(/\w/.test('\u212A'));
// CHECK-NEXT: false
print(/\w/u.test('\u212A'));
// CHECK-NEXT: false
print(/\w/i.test('\u212A'));
// CHECK-NEXT: false

// Inverted class: \W with /iu should NOT match KELVIN SIGN.
print(/\W/iu.test('\u212A'));
// CHECK-NEXT: false

// U+017F LATIN SMALL LETTER LONG S folds to 's'.
print(/\w/iu.test('\u017F'));
// CHECK-NEXT: true
print(/\w/.test('\u017F'));
// CHECK-NEXT: false

// Bracket expression with \w.
print(/[\w]/iu.test('\u212A'));
// CHECK-NEXT: true
print(/[\w]/iu.test('\u017F'));
// CHECK-NEXT: true

// Negated bracket with \W.
print(/[^\w]/iu.test('\u212A'));
// CHECK-NEXT: false
