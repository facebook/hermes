// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
  /* Some text to pad out the function so that it won't be eagerly compiled
   * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
   * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
   */
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}[1]:12:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:5:3
// CHECK-NEXT: 1 E {{.*}}:5:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}lazy-break.js[1]:5:3
// CHECK-NEXT: 1 E {{.*}}:5:3
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
