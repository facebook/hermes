/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('promise');
// CHECK-LABEL: promise

print(HermesInternal.hasPromise())
// CHECK-NEXT: true

print('all' in Promise);
// CHECK-NEXT: true

print('allSettled' in Promise);
// CHECK-NEXT: true

print('any' in Promise);
// CHECK-NEXT: true

print('finally' in Promise.prototype);
// CHECK-NEXT: true

var promise = new Promise(function(res, rej) {
  res('success!');
});

promise.then(function(message) {
  print('Resolved:', message);
});
// CHECK-NEXT: Resolved: success!

// Promise.allSettled per spec §27.2.4.2.
// Mix of fulfilled/rejected; results preserve input order with
// {status, value/reason} shape.
Promise.allSettled([
  Promise.resolve(1),
  Promise.reject('e'),
  Promise.resolve(3),
]).then(function(results) {
  results.forEach(function(r, i) {
    if (r.status === 'fulfilled') {
      print('allSettled[' + i + ']:', r.status, r.value);
    } else {
      print('allSettled[' + i + ']:', r.status, r.reason);
    }
  });
});

// Empty iterable: fulfilled with [].
Promise.allSettled([]).then(function(r) {
  print('allSettled-empty length:', r.length);
});
// CHECK-NEXT: allSettled-empty length: 0
// CHECK-NEXT: finally-fulfilled
// CHECK-NEXT: finally-rejected
// CHECK-NEXT: allSettled[0]: fulfilled 1
// CHECK-NEXT: allSettled[1]: rejected e
// CHECK-NEXT: allSettled[2]: fulfilled 3

// Promise.race per spec §27.2.4.5.
// First settled wins (fulfillment).
Promise.race([
  Promise.resolve('first'),
  Promise.reject('second'),
]).then(function(v) { print('race-fulfilled:', v); },
        function(e) { print('race-unexpected-reject:', e); });
// CHECK-NEXT: race-fulfilled: first

// First settled wins (rejection).
Promise.race([
  Promise.reject('rej-first'),
  Promise.resolve('res-second'),
]).then(function(v) { print('race-unexpected-fulfill:', v); },
        function(e) { print('race-rejected:', e); });
// CHECK-NEXT: race-rejected: rej-first

// Promise.prototype.finally per spec §27.2.5.3.
// Fulfilled: callback fires, value flows through unchanged.
Promise.resolve(42).finally(function() { print('finally-fulfilled'); })
  .then(function(v) { print('then-after-fulfilled:', v); });

// Rejected: callback fires, rejection flows through unchanged.
Promise.reject('err').finally(function() { print('finally-rejected'); })
  .then(undefined, function(e) { print('catch-after-rejected:', e); });

// Non-callable onFinally: passed through; original value preserved.
Promise.resolve('val').finally('not a function')
  .then(function(v) { print('non-callable:', v); });
// CHECK-NEXT: non-callable: val

// Throw inside finally on fulfilled: result rejected with thrown error.
Promise.resolve(1).finally(function() { throw 'oops'; })
  .then(function(v) { print('unexpected:', v); },
        function(e) { print('catch-from-throw:', e); });
// CHECK-NEXT: catch-from-throw: oops

// Non-promise return value from finally is ignored; original value flows.
Promise.resolve('orig').finally(function() { return 'ignored'; })
  .then(function(v) { print('non-promise-return:', v); });
// CHECK-NEXT: then-after-fulfilled: 42
// CHECK-NEXT: catch-after-rejected: err
// CHECK-NEXT: non-promise-return: orig

// Returned rejected promise from finally overrides original fulfillment.
Promise.resolve('orig').finally(function() {
  return Promise.reject('overrideErr');
}).then(function(v) { print('not reached:', v); },
        function(e) { print('rejected-override:', e); });
// CHECK-NEXT: rejected-override: overrideErr

// Regression: onFinally returning an object with `constructor === Promise`
// but no `.then` method must be wrapped via PromiseResolve(C, x) per spec
// §27.2.5.3.1 step 4. A previous shortcut treated such objects as
// already-Promises and called `.then` on them, throwing TypeError.
Promise.resolve('orig-fake-then').finally(function() {
  return { constructor: Promise };
}).then(function(v) { print('fake-constructor:', v); },
        function(e) { print('fake-constructor-rejected:', e && e.constructor.name); });
// CHECK-NEXT: fake-constructor: orig-fake-then

var deferred;

HermesInternal.enablePromiseRejectionTracker({
  allRejections: true,
  onUnhandled: function(id, error, p) {
    print("Unhandled:", error, "samePromise:", p === deferred);
  },
});

var rejectedPromise = new Promise(function(res, rej) {
  rej("failure!");
});

// `.then()` attaches a handler to `rejectedPromise`, which fires `_B` and
// clears its rejection entry. The rejection then propagates to the deferred
// promise that `.then()` returns; that's the promise the tracker eventually
// reports as unhandled.
deferred = rejectedPromise.then(function() {
  print('resolved');
});
// CHECK-NEXT: Unhandled: failure! samePromise: true
