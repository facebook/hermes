// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw');
// CHECK-LABEL: throw

function foo() {
  throw new Error('error1');
}

debugger;
try {
  foo();
} catch(e) {
  print('first');
  print(e.message);
  print('second');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:12:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in foo: {{.*}}:9:3
// CHECK-NEXT: Stepped to global: {{.*}}:15:3
// CHECK-NEXT: Stepped to global: {{.*}}:16:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:17:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: error1
// CHECK-NEXT: second
