/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-sema %s | %FileCheck %s
// RUN: %hermesc -Xcompile=false -dump-sema %s | %FileCheck %s

// The answers CheckImplicitReturn gives for a try statement carrying both a
// handler and a finalizer. Both RUN lines deliberately share one set of
// expectations: the compiler splits "try B catch H finally F" into nested try
// statements before the check runs and the parser entry point does not, so the
// two paths have to agree, and this fails if they ever stop agreeing.
//
// A Func line does not name its function, so each one below is followed by a
// check for a uniquely named parameter, which ties it to the function it came
// from. The first Func line is the global function holding the declarations.

// CHECK:Func loose mayReachImplicitReturn

// Nothing terminates.
function fallsThrough(pFallsThrough) {
  try { g(); } catch (e) { g(); } finally { g(); }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pFallsThrough' Parameter

// The try returns but the handler completes normally.
function catchFallsThrough(pCatchFallsThrough) {
  try { return 1; } catch (e) { g(); } finally { g(); }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pCatchFallsThrough' Parameter

// Try and catch both return and the finalizer completes normally, so the
// finalizer cannot redirect control anywhere.
function bothReturn(pBothReturn) {
  try { return 1; } catch (e) { return 2; } finally { g(); }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pBothReturn' Parameter

// A return in the finalizer overrides however the protected part completed,
// including the handler falling through.
function finallyReturns(pFinallyReturns) {
  try { g(); } catch (e) { g(); } finally { return 1; }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pFinallyReturns' Parameter

// 'break lbl' in the finalizer continues after the labeled block, so the end
// of the function is reachable even though try and catch both return.
function finallyBreaks(pFinallyBreaks) {
  lbl: {
    try { return 1; } catch (e) { return 2; } finally { break lbl; }
  }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pFinallyBreaks' Parameter

// A finalizer that only sometimes breaks reaches both the label and its own
// next statement, so it neither terminates nor unconditionally continues.
function finallyBreaksSometimes(pFinallyBreaksSometimes) {
  lbl: {
    try { return 1; } catch (e) { return 2; } finally { if (g()) break lbl; }
  }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pFinallyBreaksSometimes' Parameter
