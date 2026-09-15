/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-match -Xcompile=false -dump-sema %s \
// RUN:   | %FileCheck %s

// The answers CheckImplicitReturn gives for a Flow 'match' statement, which
// used to trip the "unhandled statement in statement list" assert instead.
// Parser mode is the only mode that reaches them, since compiling a match is
// rejected in SemanticResolver; see match-unsupported.js.
//
// A Func line does not name its function, so each one below is followed by a
// check for a uniquely named parameter, which ties it to the function it came
// from. The first Func line is the global function holding the declarations.

// CHECK:Func loose mayReachImplicitReturn

// The wildcard always runs, so one of the two returns always does.
function allReturn(pAllReturn) {
  match (pAllReturn) { 1 => { return 1; } _ => { return 2; } }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pAllReturn' Parameter

// A binding pattern accepts every value just like the wildcard does.
function binding(pBinding) {
  match (pBinding) { const a => { return a; } }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pBinding' Parameter

// 'as' renames what the inner pattern matched without narrowing it.
function asPattern(pAsPattern) {
  match (pAsPattern) { _ as a => { return a; } }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pAsPattern' Parameter

// A guard can fail however permissive the pattern is.
function guarded(pGuarded) {
  match (pGuarded) { _ if (g()) => { return 1; } }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pGuarded' Parameter

// Exhaustiveness by enumeration is not computed, so this is reported as able
// to complete normally even though the cases cover every boolean.
function enumerated(pEnumerated) {
  match (pEnumerated) { true => { return 1; } false => { return 2; } }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pEnumerated' Parameter

// No case has to run, so execution continues past the match.
function noneRequired(pNoneRequired) {
  match (pNoneRequired) { 1 => { return 1; } }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pNoneRequired' Parameter

// A case which completes normally continues past the match.
function bodyFallsThrough(pBodyFallsThrough) {
  match (pBodyFallsThrough) { _ => { g(); } }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pBodyFallsThrough' Parameter

// A return after the match still terminates the function.
function returnAfter(pReturnAfter) {
  match (pReturnAfter) { 1 => { return 1; } }
  return 2;
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pReturnAfter' Parameter

// A 'break' in a case body targets the enclosing labeled statement, so it has
// to be propagated out of the match. Without it the labeled block would
// definitely terminate.
function breakOut(pBreakOut) {
  lbl: {
    match (pBreakOut) { 1 => { break lbl; } }
    return 1;
  }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pBreakOut' Parameter

// Cases are tested in order, so the 'break outer' here is dead and must not be
// propagated: the block is left only by the wildcard's return.
function deadCase(pDeadCase) {
  outer: {
    match (pDeadCase) { _ => { return 1; } 1 => { break outer; } }
    return 2;
  }
}
// CHECK:Func loose noImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pDeadCase' Parameter

// The same case reached before the wildcard is live, so its break counts.
function liveCase(pLiveCase) {
  outer: {
    match (pLiveCase) { 1 => { break outer; } _ => { return 1; } }
    return 2;
  }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pLiveCase' Parameter

// A guard makes the earlier case refutable, so what follows it stays live.
function guardedThenLive(pGuardedThenLive) {
  outer: {
    match (pGuardedThenLive) { _ if (g()) => { return 1; } 1 => { break outer; } }
    return 2;
  }
}
// CHECK:Func loose mayReachImplicitReturn
// CHECK:Decl %d.{{[0-9]+}} 'pGuardedThenLive' Parameter
