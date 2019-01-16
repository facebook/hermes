// RUN: (! %hdb %s --break-at-start < %s.debug 2>&1) | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// CHECK: Continuing execution

try { throw new Error('asdf') } catch (e) { print('caught', e); }
// CHECK-NEXT: caught Error: asdf

function foo() {
  throw new Error('asdf');
}
foo();
// CHECK-NEXT: Break on exception in foo: {{.*}}:10:3
// CHECK-NEXT: > 0: foo: {{.*}}:10:3
// CHECK-NEXT:   1: global: {{.*}}:12:4
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: JavaScript terminated via uncaught exception: asdf

// CHECK:      Error: asdf
// CHECK-NEXT:     at foo ({{.*}}:10:18)
// CHECK-NEXT:     at global ({{.*}}:12:4)
