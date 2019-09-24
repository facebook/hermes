// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc %s 2>&1 ) | %FileCheck --match-full-lines %s

function *foo() { for (yield of x) print(1) }
// CHECK: {{.*}}:8:24: error: invalid assignment left-hand side
// CHECK: function *foo() { for (yield of x) print(1) }
// CHECK:                        ^~~~~
