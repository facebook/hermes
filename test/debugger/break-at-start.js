// RUN: %hdb %s < %s.debug --break-after 0 | %FileCheck --match-full-lines %s
// REQUIRES: debugger

0;

// CHECK: Interrupted in global: {{.*}}
// CHECK-NEXT: Hello from the debugger
