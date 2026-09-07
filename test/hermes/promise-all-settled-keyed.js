/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Per await-dictionary proposal §27.2.4.2 Promise.allSettledKeyed
// https://tc39.es/proposal-await-dictionary/#sec-promise.allsettledkeyed

print('promise-all-settled-keyed');
// CHECK-LABEL: promise-all-settled-keyed
print('allSettledKeyed' in Promise);
// CHECK-NEXT: true
print('typeof Promise.allSettledKeyed:', typeof Promise.allSettledKeyed);
// CHECK-NEXT: typeof Promise.allSettledKeyed: function

// Happy path: each key maps to {status, value/reason}.
var symbol = Symbol('key');
var result = Promise.allSettledKeyed({
  a: Promise.resolve(1),
  b: Promise.reject('2'),
  // Nested properties are not processed; this whole object is treated as
  // an already-resolved value and returned as-is inside the settled result.
  nested: {
    c: Promise.resolve(3),
  },
  // Symbol properties are processed.
  [symbol]: Promise.resolve(true)
}).then(object => {
  print('a:', object.a.status, object.a.value);
  print('b:', object.b.status, object.b.reason);
  print('nested:', object.nested.status, object.nested.value.c instanceof Promise);
  print('symbol:', object[symbol].status, object[symbol].value);
});
print('result instance:', result instanceof Promise);
// CHECK-NEXT: result instance: true

// Subclass dispatch: returned promise is an instance of the subclass.
class MyPromise extends Promise {}
var wrSub = MyPromise.allSettledKeyed({});
print('subclass instance:', wrSub instanceof MyPromise);
// CHECK-NEXT: subclass instance: true
var wrSubCall = Promise.allSettledKeyed.call(MyPromise, {});
print('subclass call instance:', wrSubCall instanceof MyPromise);
// CHECK-NEXT: subclass call instance: true

try {
  Promise.allSettledKeyed.call(undefined, {});
  print('non-constructor: no throw');
} catch (e) {
  print('non-constructor:', e.constructor.name);
}
// CHECK-NEXT: non-constructor: TypeError

// After microtask
// CHECK-NEXT: a: fulfilled 1
// CHECK-NEXT: b: rejected 2
// CHECK-NEXT: nested: fulfilled true
// CHECK-NEXT: symbol: fulfilled true
