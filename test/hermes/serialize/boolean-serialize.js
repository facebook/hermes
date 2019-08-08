// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";
print('Number');
// CHECK-LABEL: Number
print(Boolean());
// CHECK-NEXT: false
print(Boolean(true), Boolean(false));
// CHECK-NEXT: true false
print(new Boolean(true), new Boolean(false));
// CHECK-NEXT: true false
print(true.toString(), false.toString());
// CHECK-NEXT: true false
print(true.valueOf(), false.valueOf());
// CHECK-NEXT: true false
