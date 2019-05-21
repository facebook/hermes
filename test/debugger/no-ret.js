// RUN: %hdb --break-at-start %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

 for (;;);

// CHECK: Break on script load in global: {{.*}}:4:2
// CHECK: Stepped to global: {{.*}}:4:2
// CHECK: Stepped to global: {{.*}}:4:2
