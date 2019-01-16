// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
try {
  throw 'asdf';
} catch (e) {
  print('caught');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:4:1
// CHECK-NEXT: Stepped to global: {{.*}}:6:3
// CHECK-NEXT: Stepped to global: {{.*}}:8:3
// CHECK-NEXT: caught
