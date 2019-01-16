// RUN: %hdb --break-at-start %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// CHECK: Break on script load in global: {{.*}}:5:1
print("Hello hdb!");
debugger;
print("Good bye hdb!");
// CHECK-NEXT: Hello hdb!
// CHECK-NEXT: Break on 'debugger' statement in global: {{.*}}:6:1
// CHECK-NEXT: Good bye hdb!
