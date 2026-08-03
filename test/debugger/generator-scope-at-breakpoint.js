/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --Xes6-block-scoping %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

// Regression test: a breakpoint on a yield line must resolve to real user
// code, not the generator's synthetic resume prologue. The lowering pass used
// to stamp the prologue (which runs at the lowest offsets on every resume) with
// the source line of the last yield, so a breakpoint on that line paused at the
// scope-less function entry and no variables were visible.
//
// Uses `info variables` (not `exec`) and sets no breakpoint after any eval, to
// avoid an unrelated stale-eval-module issue in breakpoint resolution.
(function () {
  function* gen() {
    let outer = 20;
    for (let i = 0; i < 2; i++) {
      let v3 = 30;
      yield i;
      yield v3 + outer; // Breakpoint set on this line (the last yield).
    }
  }
  let g = gen();
  debugger;
  g.next();
  g.next();
  g.next();
})();
// CHECK: Break on 'debugger' statement in {{.*}}
// CHECK-NEXT: Set breakpoint {{[0-9]+}} at {{.*}}:25:7
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: Break on breakpoint {{[0-9]+}} in gen: {{.*}}:25:7
// The scope must be populated with the in-scope variables at the yield.
// CHECK-DAG: {{.*}}v3 = 30
// CHECK-DAG: {{.*}}outer = 20
// CHECK-DAG: {{.*}}i = 0
// CHECK: Continuing execution
