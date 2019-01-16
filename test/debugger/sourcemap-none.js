// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
// CHECK: Break on 'debugger' statement in global:{{.*}}
// CHECK: Source map not found for file
// CHECK-NEXT: Continuing execution
