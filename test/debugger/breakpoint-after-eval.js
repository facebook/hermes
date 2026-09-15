/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// Regression test: evaluating an expression in a frame (`exec`) must not leave
// a runtime module whose source buffer aliases the debugger's transient command
// text. The eval used to borrow (not copy) the command string, so after that
// debugger session resumed, a breakpoint resolved at a *later* pause -- which
// walks the source of every runtime module, including the stale eval module --
// read freed memory (ASan: stack-use-after-return in
// SourceErrorManager::findForCoordsImpl). This test evals at one pause, resumes,
// then sets a by-line breakpoint at a later pause; it crashes under ASan without
// the fix.
function foo() {
  var x = 1;
  debugger;
  var y = 2;
  debugger;
  return x + y;
}
print(foo());
print('DONE');
// CHECK: Break on 'debugger' statement in foo: {{.*}}:22:3
// CHECK-NEXT: 1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on 'debugger' statement in foo: {{.*}}:24:3
// CHECK-NEXT: Set breakpoint 1 at {{.*}}:25:{{[0-9]+}}
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint 1 in foo: {{.*}}:25:{{[0-9]+}}
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: 3
// CHECK-NEXT: DONE
