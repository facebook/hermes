/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy -Xes6-promise %s | %FileCheck --match-full-lines %s

// Noted that the process of resolving Promise is asynchronous,
// and deeper promise chain will take more microtask ticks to complete.
//
// Hence, the running of tests are ordered from taking less ticks to more ticks
// to make printings conforming with the order of comments in the source code.

print('async function');
// CHECK-LABEL: async function
// CHECK-NEXT: 0

// A pseudo task to simulate an async operation.
function task(v) {
  return new Promise((resolve, _) => resolve(v));
}

// --- Fundamentals --- //

async function empty() {};

async function simpleReturn() {
  return 1;
}

async function adopted() {
  var x = Promise.resolve(1)  
  return x;
}

async function simpleAwait() {
  var x = await 2;
  return x;
}

async function sequencing() {
  var t1 = await 1;
  var t2 = await task(t1 + 1);
  var t3 = await task(t2 + 1);
  // 3rd tick
  print(t3, "sequencing");
  return t3;
}

// --- Parameters --- //

async function simpleParam(x) {
  var x = await x;
  return x;
}

async function nonSimpleDefaultAssign(x=2) {
  var x = await x;
  return x;
}

async function nonSimpleArrayDestructuring([x]) {
  var x = await x;
  return x;
}

async function nonSimpleObjDestructuring({x}) {
  var x = await x;
  return x;
}

async function restParam(...args) {
  var x = await args[2];
  return x;
}

// --- Error handling --- //

async function simpleThrow() {
  throw 1;
}

async function tryCatch() {
  try {
    throw 1;
  } catch (e) {
    // same tick
    throw e;
  }
}

async function tryFinally() {
  try {
    var t1 = await 1;
    var t2 = await task(t1 + 1)
    var t3 = await task(t2 + 1)
    print(t3, "tryFinally");
  } finally {
    // 3rd tick
    print(3, "tryFinally finally");
  }
}

async function tryFinallyReturn() {
  try {
    var t1 = await 1;
    var t2 = await task(t1 + 1)
    var t3 = await task(t2 + 1)
    // 3rd tick
    print(t3, "tryFinallyReturn");
  } finally {
    // 3rd tick
    print(3, "tryFinallyReturn finally");
    return t3;
  }
}

async function tryCatchFinally() {
  try {
    var t1 = await 1;
    var t2 = await task(t1 + 1)
    var t3 = await task(t2 + 1)
    // 3rd tick
    print(t3, "tryCatchFinally");
    throw t3;
  } catch (e) {
    // 3rd tick
    print(t3, "tryCatchFinally catch");
    throw e;
  } finally {
    // 3rd tick
    print(3, "tryCatchFinally finally");
  }
}

// --- One ticks --- //

empty().then(v => print(v, "empty"));
// CHECK-NEXT: undefined empty
simpleReturn().then(v => print(v, "simpleReturn"));
// CHECK-NEXT: 1 simpleReturn
adopted().then(v => print(v, "adopted"));
// CHECK-NEXT: 1 adopted
tryCatch().catch(e => print(e, "tryCatch"));
// CHECK-NEXT: 1 tryCatch
simpleThrow().catch(e => print(e, "simpleThrow"));
// CHECK-NEXT: 1 simpleThrow

// --- Two ticks --- //

simpleAwait().then(v => print(v, "simpleAwait"));
// CHECK-NEXT: 2 simpleAwait
simpleParam(2).then(v => print(v, "simpleParam"));
// CHECK-NEXT: 2 simpleParam
nonSimpleDefaultAssign(2).then(v => print(v, "nonSimpleDefaultAssign"));
// CHECK-NEXT: 2 nonSimpleDefaultAssign
nonSimpleArrayDestructuring([2]).then(v => print(v, "nonSimpleArrayDestructuring"));
// CHECK-NEXT: 2 nonSimpleArrayDestructuring
nonSimpleObjDestructuring({x:2}).then(v => print(v, "nonSimpleObjDestructuring"));
// CHECK-NEXT: 2 nonSimpleObjDestructuring
restParam(0, 1, 2).then(v => print(v, "restParam"));
// CHECK-NEXT: 2 restParam

// --- Three ticks --- //
// print inside async function body goes into thc 3rd tick,
// print inside `then`/`catch` callback goes into the 4th tick.

sequencing().then(v => print(v+1, "sequencing cb"));
// CHECK-NEXT: 3 sequencing
tryFinally().then(v => print(v, "tryFinally cb"))
// CHECK-NEXT: 3 tryFinally
// CHECK-NEXT: 3 tryFinally finally
tryFinallyReturn().then(v => print(v + 1, "tryFinallyReturn cb"));
// CHECK-NEXT: 3 tryFinallyReturn
// CHECK-NEXT: 3 tryFinallyReturn finally
tryCatchFinally().catch(v => print(v + 1, "tryCatchFinally cb"));
// CHECK-NEXT: 3 tryCatchFinally
// CHECK-NEXT: 3 tryCatchFinally catch
// CHECK-NEXT: 3 tryCatchFinally finally

// --- Four ticks --- //

// CHECK-NEXT: 4 sequencing cb
// CHECK-NEXT: undefined tryFinally cb
// CHECK-NEXT: 4 tryFinallyReturn cb
// CHECK-NEXT: 4 tryCatchFinally cb

// --- Zero ticks --- //

print("0")
