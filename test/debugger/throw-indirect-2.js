// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw indirect 2');
// CHECK-LABEL: throw indirect 2

function foo() {
  throw new Error('error1');
}

debugger;
try {
  foo.call();
} catch(e) {
  print('first');
  print(e.message);
  print('second');
}

try {
  throw new Error('uncaught');
} catch(e) {}

// CHECK: Break on 'debugger' statement in global: {{.*}}:12:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in foo: {{.*}}:9:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: first
// CHECK-NEXT: error1
// CHECK-NEXT: second
// CHECK-NEXT: Break on exception in global: {{.*}}:22:3
// CHECK-NEXT: Continuing execution
