// RUN: ( ! %hdb %s < %s.debug 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
throw 'err(asdf)';

// CHECK: Break on 'debugger' statement in global: {{.*}}:4:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Stepped to global: {{.*}}:5:1
// CHECK-NEXT: Break on exception in global: {{.*}}:5:1
// CHECK-NEXT: JavaScript terminated via uncaught exception: err(asdf)
