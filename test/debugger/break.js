// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('break');
// CHECK-LABEL: break

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
// CHECK-NEXT: Set breakpoint 1 at {{.+}}:9:3
// CHECK-NEXT: Invalid or duplicate breakpoint not set
// CHECK-NEXT: Set breakpoint 2 at {{.+}}:10:3
// CHECK-NEXT: Deleted breakpoint 2
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:9:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: third
