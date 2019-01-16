// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('first');
debugger;
print('second');
debugger;
print('third');

// CHECK: Break on script load in global: {{.*}}:4:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:5:1
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:7:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: Break on breakpoint 1 in global: {{.*}}:5:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
// CHECK-NEXT: Break on breakpoint 2 in global: {{.*}}:7:1
// CHECK-NEXT: Stepped to global: {{.*}}:8:1
// CHECK-NEXT: third
