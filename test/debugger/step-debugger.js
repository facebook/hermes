// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step debugger');
// CHECK-LABEL: step debugger

debugger;
print('first');
print('second');

// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}7:1
// CHECK-NEXT: Stepped to global: {{.*}}8:1
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}9:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: second
