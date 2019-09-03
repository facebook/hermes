// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";
print('Number');
// CHECK-LABEL: Number
print(Number())
// CHECK-NEXT: 0
print(Number(1))
// CHECK-NEXT: 1
var n = new Number(123);
print(n, n.__proto__ === Number.prototype);
// CHECK-NEXT: 123 true

print('constants');
// CHECK-LABEL: constants
print(Number.MAX_VALUE);
// CHECK-NEXT: 1.7976931348623157e+308

print('toPrecision');
// CHECK-LABEL: toPrecision
print((-1234.567).toPrecision(25));
// CHECK-NEXT: -1234.567000000000007275958

print('isFinite');
// CHECK-LABEL: isFinite
print(Number.isFinite(3));
// CHECK-NEXT: true

print('isInteger');
// CHECK-LABEL: isInteger
print(Number.isInteger('asdf'));
// CHECK-NEXT: false
