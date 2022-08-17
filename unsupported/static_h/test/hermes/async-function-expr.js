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

print('async function expression');
// CHECK-LABEL: async function expression
// CHECK-NEXT: 0

// A pseudo task to simulate an async operation.
function task(v) {
  return new Promise((resolve, _) => resolve(v));
}


// --- One ticks --- //

(async function () {})().then(v => print(v));
// CHECK-NEXT: undefined

(async function simpleReturn() {
  return 1;
})().then(v => print(v));
// CHECK-NEXT: 1

(async function adopted() {
  var x = Promise.resolve(1)
  return x;
})().then(v => print(v));
// CHECK-NEXT: 1

(async function tryCatch() {
  try {
    throw 0;
  } catch (e) {
    throw e+1;
  }
})().catch(v => print(v));
// CHECK-NEXT: 1

(async function simpleThrow() {
  throw 1;
})().catch(v => print(v));
// CHECK-NEXT: 1


// --- Two ticks --- //

(async function simpleParam(x) {
  var x = await x;
  return x;
})(2).then(v => print(v));
// CHECK-NEXT: 2

(async function simpleAwait() {
  var x = await 2;
  return x;
})().then(v => print(v));
// CHECK-NEXT: 2

(async function nonSimpleArrayDestructuring([x]) {
  var x = await x;
  return x;
})([2]).then(v => print(v));
// CHECK-NEXT: 2


// --- Three ticks --- //
// print inside async function body goes into thc 3rd tick,
// print inside `then`/`catch` callback goes into the 4th tick.

(async function sequencing() {
  var t1 = await 1;
  var t2 = await task(t1 + 1);
  var t3 = await task(t2 + 1);
  // 3rd tick
  print(t3);
  return t3;
})().then(v => print(v+1));
// CHECK-NEXT: 3

(async function tryCatchFinally() {
  try {
    var t1 = await 1;
    var t2 = await task(t1 + 1)
    var t3 = await task(t2 + 1)
    // 3rd tick
    print(t3);
    throw t3;
  } catch (e) {
    // 3rd tick
    print(t3);
    throw e;
  } finally {
    // 3rd tick
    print("tryCatchFinally");
  }
})().catch(v => print(v+1));
// CHECK-NEXT: 3
// CHECK-NEXT: 3
// CHECK-NEXT: tryCatchFinally


// --- Four ticks --- //
// CHECK-NEXT: 4
// CHECK-NEXT: 4


// --- Zero ticks --- //

print("0")
