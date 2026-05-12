/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Regression: RegExp must not reuse the per-regexp groupNameMappings_
// HiddenClass for the .groups (or .indices.groups) object. With at least
// kDictionaryThreshold + 1 named groups that class is in dictionary mode,
// and dictionary classes are owned by exactly one object and mutated in
// place; sharing one lets a mutation on one match's .groups corrupt
// another (out-of-bounds property storage access).

// kDictionaryThreshold + 1: the slow-path object transitions to a dictionary.
var N = 65;

var src = '', input = '';
for (var i = 0; i < N; i++) { src += '(?<g' + i + '>.)'; input += 'x'; }
var re = new RegExp(src, 'd');
var m1 = re.exec(input), m2 = re.exec(input);
delete m1.groups.g1;
print(Object.keys(m2.groups).length, m2.groups.g1);
// CHECK: 65 x
m1.groups.EXTRA = 'BAD';
print(m2.groups.EXTRA);
// CHECK-NEXT: undefined

// Same for the .indices.groups path.
print(Object.keys(m1.indices.groups).length);
// CHECK-NEXT: 65
delete m1.indices.groups.g1;
print(Object.keys(m2.indices.groups).length, m2.indices.groups.g1[0]);
// CHECK-NEXT: 65 1
