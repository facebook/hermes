// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('conditional break none');
// CHECK-LABEL: conditional break none
debugger;

print('first');
print('second');
print('third');

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:6:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:8:1 if throw new Error()
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:9:1 if 1 +
// CHECK-NEXT: Set breakpoint 3 at {{.*}}:10:1 if undefined
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: second
// CHECK-NEXT: third
