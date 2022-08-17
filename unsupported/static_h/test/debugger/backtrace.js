/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

// CHECK: Break on 'debugger' statement in baz: {{.*}}:12:3
// CHECK-NEXT: > 0: baz: {{.*}}:12:3
// CHECK-NEXT:   1: (native)
// CHECK-NEXT:   2: bar: {{.*}}:17:11
// CHECK-NEXT:   3: foo: {{.*}}:22:6
// CHECK-NEXT:   4: global: {{.*}}:25:4
// CHECK-NEXT: Continuing execution

// CHECK: Break on 'debugger' statement in baz: {{.*}}:12:3
// CHECK-NEXT: > 0: baz: {{.*}}:12:3
// CHECK-NEXT:   1: bar: {{.*}}:18:17
// CHECK-NEXT:   2: foo: {{.*}}:22:6
// CHECK-NEXT:   3: global: {{.*}}:25:4
// CHECK-NEXT: Continuing execution

function first() {
  debugger;
  return;
}

function second() {
  first();
}

function third() {
  second();
}

first.displayName = "1st";
second.displayName = "2nd";
third.displayName = "3rd";

third();

// CHECK: Break on 'debugger' statement in 1st: {{.*}}:43:3
// CHECK-NEXT: > 0: 1st: {{.*}}:43:3
// CHECK-NEXT:   1: 2nd: {{.*}}:48:8
// CHECK-NEXT:   2: 3rd: {{.*}}:52:9
// CHECK-NEXT:   3: global: {{.*}}:59:6
