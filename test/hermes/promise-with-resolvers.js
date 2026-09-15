/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Per ES2025 §27.2.4.9 Promise.withResolvers:
//   1. Let promiseCapability be ? NewPromiseCapability(this).
//   2. Return { promise, resolve, reject } from the capability.
//
// NewPromiseCapability + GetCapabilitiesExecutor (§27.2.1.5 /
// §27.2.1.5.1) enforce:
//   - IsConstructor(this) (TypeError if not).
//   - The capability executor is called at most once with
//     non-undefined args (TypeError otherwise — steps 3-4).
//   - resolve/reject are functions after construction returns
//     (TypeError otherwise — §27.2.1.5 step 7-8).

print('promise-with-resolvers');
// CHECK-LABEL: promise-with-resolvers
print('withResolvers' in Promise);
// CHECK-NEXT: true
print('typeof Promise.withResolvers:', typeof Promise.withResolvers);
// CHECK-NEXT: typeof Promise.withResolvers: function

// Happy path: shape of the returned object.
var wr = Promise.withResolvers();
print('typeof promise:', typeof wr.promise);
print('typeof resolve:', typeof wr.resolve);
print('typeof reject:', typeof wr.reject);
print('promise instanceof Promise:', wr.promise instanceof Promise);
// CHECK-NEXT: typeof promise: object
// CHECK-NEXT: typeof resolve: function
// CHECK-NEXT: typeof reject: function
// CHECK-NEXT: promise instanceof Promise: true

// Subclass dispatch: returned promise is an instance of the subclass.
class MyPromise extends Promise {}
var wrSub = MyPromise.withResolvers();
print('subclass instance:', wrSub.promise instanceof MyPromise);
// CHECK-NEXT: subclass instance: true
var wrSubCall = Promise.withResolvers.call(MyPromise);
print('subclass call instance:', wrSubCall.promise instanceof MyPromise);
// CHECK-NEXT: subclass call instance: true

// Non-constructor `this` → TypeError per IsConstructor check.
try {
  Promise.withResolvers.call(undefined);
  print('non-constructor: no throw');
} catch (e) {
  print('non-constructor:', e.constructor.name);
}
// CHECK-NEXT: non-constructor: TypeError

// Constructor whose executor is never called → resolve/reject would
// be undefined; spec mandates TypeError from the IsCallable check
// (§27.2.1.5 step 7-8).
class NoCallP {
  constructor(_executor) { /* never call executor */ }
}
try {
  Promise.withResolvers.call(NoCallP);
  print('no-call-executor: no throw');
} catch (e) {
  print('no-call-executor:', e.constructor.name);
}
// CHECK-NEXT: no-call-executor: TypeError

// Microtask-observable cases come last (after the sync prints above)
// because the spec-mandated PromiseReactionJob hop drains them after
// the script's sync code returns.

// resolve/reject control the promise.
var r1 = Promise.withResolvers();
r1.resolve('resolved-value');
r1.promise.then(function(v) { print('resolved:', v); });

var r2 = Promise.withResolvers();
r2.reject('rejected-reason');
r2.promise.then(undefined, function(e) { print('rejected:', e); });

// Constructor that calls the capability executor twice — per
// §27.2.1.5.1 steps 3-4 the second call must throw TypeError. The
// standard Promise super-constructor catches that throw and rejects
// the promise (§27.2.3.1 step 9), so the TypeError is observable
// via the returned promise's rejection rather than synchronously.
class DoubleCallP extends Promise {
  constructor(executor) {
    super(function (res, rej) {
      executor(res, rej);
      executor(res, rej);  // second call → TypeError → promise rejected
    });
  }
}
var wrDc = DoubleCallP.withResolvers();
wrDc.promise.then(
  function(v) { print('double-call: unexpected resolve:', v); },
  function(e) { print('double-call:', e.constructor.name); }
);
// CHECK-NEXT: resolved: resolved-value
// CHECK-NEXT: rejected: rejected-reason
// CHECK-NEXT: double-call: TypeError
