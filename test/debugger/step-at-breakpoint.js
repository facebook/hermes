// RUN: %hdb --break-at-start < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

var a = 1;
var b = a + 3;
var c = a * b;

// CHECK: Break on script load in global: {{.*}}:4:1
// CHECK: Set breakpoint 1 at {{.*}}:5:9
// CHECK: Continuing execution
// CHECK: Break on breakpoint 1 in global: {{.*}}:5:9
// CHECK: Stepped to global: {{.*}}:6:9
