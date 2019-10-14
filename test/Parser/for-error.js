// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

for (;false;) function foo() {}
// CHECK: {{.*}}:8:15: error: declaration not allowed as expression statement
// CHECK: for (;false;) function foo() {}
// CHECK:               ^~~~~~~~

for (;false;) class bar {}
// CHECK: {{.*}}:13:15: error: declaration not allowed as expression statement
// CHECK: for (;false;) class bar {}
// CHECK:               ^~~~~

for (;false;) let [x] = 3;
// CHECK: {{.*}}:18:15: error: ambiguous 'let [': either a 'let' binding or a member expression
// CHECK: for (;false;) let [x] = 3;
// CHECK:               ^~~~~
