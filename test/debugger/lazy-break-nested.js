// RUN: %hdb --lazy %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
  /**
   * Some text to pad out the function so that it won't be eagerly compiled
   * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
   * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
   */

  function bar() {
    print('bar called');
    /**
     * Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }

  function baz() {
    print('baz called');
    /**
     * Some text to pad out the function so that it won't be eagerly compiled
     * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
     * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
     */
  }

  bar();
  baz();
}

debugger;
foo();

// CHECK: Break on 'debugger' statement in global: {{.*}}:34:1
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:13:5
// CHECK-NEXT: Set breakpoint 2 at {{.*}}:22:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: foo called
// CHECK-NEXT: Break on breakpoint 1 in bar: {{.*}}lazy-break-nested.js[1]:13:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: bar called
// CHECK-NEXT: Break on breakpoint 2 in baz: {{.*}}lazy-break-nested.js[1]:22:5
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: baz called
