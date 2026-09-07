/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Per await-dictionary proposal §27.2.4.1 Promise.allKeyed
// https://tc39.es/proposal-await-dictionary/#sec-promise.allkeyed

print('promise-all-keyed');
// CHECK-LABEL: promise-all-keyed
print('allKeyed' in Promise);
// CHECK-NEXT: true
print('typeof Promise.allKeyed:', typeof Promise.allKeyed);
// CHECK-NEXT: typeof Promise.allKeyed: function

// Happy path: shape of the returned object.
var symbol = Symbol('key');
var result = Promise.allKeyed({
  a: Promise.resolve(1),
  b: Promise.resolve('2'),
  // Nested properties are not processed; this whole object is treated as
  // an already-resolved value and returned as-is
  nexted: {
    c: Promise.resolve(3),
  },
  // Symbol properties are processed
  [symbol]: Promise.resolve(true)
}).then(object => {
  print('typeof a:', typeof object.a);
  print('typeof b:', typeof object.b);
  print('typeof nexted.c:', object.nexted.c instanceof Promise);
  print('typeof symbol:', typeof object[symbol]);
});
print('result instance:', result instanceof Promise);
// CHECK-NEXT: result instance: true

// Subclass dispatch: returned promise is an instance of the subclass.
class MyPromise extends Promise {}
var wrSub = MyPromise.allKeyed({});
print('subclass instance:', wrSub instanceof MyPromise);
// CHECK-NEXT: subclass instance: true
var wrSubCall = Promise.allKeyed.call(MyPromise, {});
print('subclass call instance:', wrSubCall instanceof MyPromise);
// CHECK-NEXT: subclass call instance: true

try {
  Promise.allKeyed.call(undefined, {});
  print('non-constructor: no throw');
} catch (e) {
  print('non-constructor:', e.constructor.name);
}
// CHECK-NEXT: non-constructor: TypeError


// After microtask
// CHECK-NEXT: typeof a: number
// CHECK-NEXT: typeof b: string
// CHECK-NEXT: typeof nexted.c: true
// CHECK-NEXT: typeof symbol: boolean
