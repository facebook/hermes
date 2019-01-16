// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('print breakpoints');
// CHECK-LABEL: print breakpoints

function foo() {
  print('first');
  print('second');
  print('third');
}

debugger;
foo();

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:8:3
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:9:3
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:10:3
// CHECK-NEXT: Disabled breakpoint 1
// CHECK-NEXT: Disabled breakpoint 2
// CHECK-NEXT: 1 D {{.*}}print-breakpoints.js:8:3
// CHECK-NEXT: 2 D {{.*}}print-breakpoints.js:9:3
// CHECK-NEXT: 3 E {{.*}}print-breakpoints.js:10:3
// CHECK-NEXT: Deleted breakpoint 1
// CHECK-NEXT: Deleted breakpoint 3 
// CHECK-NEXT: 2 D {{.*}}print-breakpoints.js:9:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: third
