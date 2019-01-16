// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function fact(n) {
  if (n < 2) {
    return 1;
  }
  return n * fact(n - 1);
}

debugger;
fact(4);

// CHECK: Break on 'debugger' statement in global: {{.*}}:11:1
// CHECK-NEXT: Stepped to global: {{.*}}:12:1
// CHECK-NEXT: Stepped to fact: {{.*}}:5:7
// CHECK-NEXT: Stepped to global: {{.*}}:12:5
// CHECK-NEXT: Continuing execution
