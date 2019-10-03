// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: cat %s | %repl -prompt "" 2>&1 | %FileCheck --match-full-lines %s

"assignment"
// CHECK-LABEL: "assignment"
x = 1
// CHECK-NEXT: 1
y = x + 1
// CHECK-NEXT: 2
y = x / 2
// CHECK-NEXT: 0.5
var z = 3
// CHECK-NEXT: undefined
z
// CHECK-NEXT: 3
