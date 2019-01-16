// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw');
// CHECK-LABEL: throw

debugger;
try {
  throw new Error('asdf');
} catch(e) {
  print('caught it');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:7:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in global: {{.*}}:9:3
// CHECK-NEXT: Stepped to global: {{.*}}:11:3
// CHECK-NEXT: caught it
