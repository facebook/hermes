// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;

// CHECK: Break on 'debugger' statement in global:{{.*}}
// CHECK: These hdb commands are defined internally. {{.*}}
// CHECK: Modifies selected frame {{.*}}
// CHECK: Not a valid command. {{.*}}
