// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var x = 1;
debugger;
print(x);

// CHECK: Break on 'debugger' statement in global: {{.*}}:5:1
// CHECK-NEXT: 3
// CHECK-NEXT: Stepped to global: {{.*}}:6:1

