// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('hello');
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}:8:1
// CHECK-NEXT: Stepped to global: {{.*}}:9:1
// CHECK-NEXT: Stepped to foo: {{.*}}:5:3
// CHECK-NEXT: hello
// CHECK-NEXT: Stepped to global: {{.*}}:9:4
