// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function baz(x, y) {
  debugger;
  return x + y;
}

function bar() {
  baz.call(undefined, 1,2);
  baz.bind(null)(1, 2);
}

function foo() {
  bar();
}

foo();

// CHECK: Break on 'debugger' statement in baz: {{.*}}:10:3
// CHECK-NEXT: > 0: baz: {{.*}}:10:3
// CHECK-NEXT:   1: (native)
// CHECK-NEXT:   2: bar: {{.*}}:15:11
// CHECK-NEXT:   3: foo: {{.*}}:20:6
// CHECK-NEXT:   4: global: {{.*}}:23:4
// CHECK-NEXT: Continuing execution

// CHECK: Break on 'debugger' statement in baz: {{.*}}:10:3
// CHECK-NEXT: > 0: baz: {{.*}}:10:3
// CHECK-NEXT:   1: bar: {{.*}}:16:17
// CHECK-NEXT:   2: foo: {{.*}}:20:6
// CHECK-NEXT:   3: global: {{.*}}:23:4
// CHECK-NEXT: Continuing execution
