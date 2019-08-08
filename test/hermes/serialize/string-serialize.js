// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

"use strict";
print('String');
// CHECK-LABEL: String
print('empty', String());
// CHECK-NEXT: empty
print(String('asdf'), String('asdf').length);
// CHECK-NEXT: asdf 4
var s = new String('asdf');
print(s, s.toString(), s.valueOf(), s.length, s.__proto__ === String.prototype);
// CHECK-NEXT: asdf asdf asdf 4 true
var desc = Object.getOwnPropertyDescriptor(s, 'length');
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false
print(String('a') === String('a'), new String('a') === new String('a'));
// CHECK-NEXT: true false

// Test some string native functions
print(String.fromCharCode(0xff0048, 0xe8, 114, 109, 101, 115));
// CHECK-NEXT: Hèrmes
print(String.fromCodePoint(65));
// CHECK-NEXT: A
print('a'.repeat(3));
// CHECK-NEXT: aaa
print('abcabc'.search(/ca/));
// CHECK-NEXT: 2
