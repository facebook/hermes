// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('throw');
// CHECK-LABEL: throw

function bar() {
  throw new Error('asdf');
}

function foo() {
  bar();
}

debugger;
try {
  foo();
} catch(e) {
  print('first');
  print('second');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:15:1
// CHECK-NEXT: Set pauseOnThrow: all errors
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on exception in bar: {{.*}}:8:3
// CHECK-NEXT: Stepped to global: {{.*}}:18:3
// CHECK-NEXT: Stepped to global: {{.*}}:19:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:20:3
// CHECK-NEXT: second
