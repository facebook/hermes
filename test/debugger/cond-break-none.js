// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('conditional break none');
// CHECK-LABEL: conditional break none
debugger;

print('first');
print('second');
print('third');

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:11:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:13:1 if throw new Error()
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:14:1 if 1 +
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:15:1 if undefined
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: third
