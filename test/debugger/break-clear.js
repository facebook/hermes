// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('break-clear');
// CHECK-LABEL: break-clear

function foo() {
  print('first');
  print('second');
  print('third');
}

function bar() {
  debugger;
  foo();
}

bar();
// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}:14:3
// CHECK-NEXT: Set breakpoint 1 at {{.+}}:8:3
// CHECK-NEXT: Set breakpoint 2 at {{.+}}:9:3
// CHECK-NEXT: Set breakpoint 3 at {{.+}}:10:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:8:3
// CHECK-NEXT: Deleted all breakpoints
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: third
