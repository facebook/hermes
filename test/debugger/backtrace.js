// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function baz(x, y) {
  debugger;
  return x + y;
}

function bar() {
  baz.call(undefined, 1,2);
}

function foo() {
  bar();
}

foo();

// CHECK: Break on 'debugger' statement in baz: {{.*}}:5:3
// CHECK-NEXT: > 0: baz: {{.*}}:5:3
// CHECK-NEXT:   1: (native)
// CHECK-NEXT:   2: bar: {{.*}}:10:11
// CHECK-NEXT:   3: foo: {{.*}}:14:6
// CHECK-NEXT:   4: global: {{.*}}:17:4
// CHECK-NEXT: Continuing execution
