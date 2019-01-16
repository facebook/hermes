// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('enable disable');
// CHECK-LABEL: enable disable

function foo() {
  print('first');
  print('second');
}

debugger;
foo();
debugger;
foo();
debugger;
foo();

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:12:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:8:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:8:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:14:1
// CHECK-NEXT: Disabled breakpoint 1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:16:1
// CHECK-NEXT: Enabled breakpoint 1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:8:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
