// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('break loop');
// CHECK-LABEL: break loop

function foo() {
  for (var i = 0; i < 3; ++i) {
    print('first');
    print('second');
  }
}

function bar() {
  debugger;
  foo();
}

bar();

// CHECK-NEXT: Break on 'debugger' statement in bar: {{.*}}:15:3
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:10:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:10:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:10:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:10:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
