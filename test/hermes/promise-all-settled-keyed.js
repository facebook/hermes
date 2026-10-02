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
// CHECK: true
print('typeof Promise.allSettledKeyed:', typeof Promise.allSettledKeyed);
// CHECK: typeof Promise.allSettledKeyed: function

// Subclass dispatch: returned promise is an instance of the subclass.
class MyPromise extends Promise {}
var wrSub = MyPromise.allSettledKeyed({});
print('subclass instance:', wrSub instanceof MyPromise);
// CHECK: subclass instance: true
var wrSubCall = Promise.allSettledKeyed.call(MyPromise, {});
print('subclass call instance:', wrSubCall instanceof MyPromise);
// CHECK: subclass call instance: true

try {
  Promise.allSettledKeyed.call(undefined, {});
  print('non-constructor: no throw');
} catch (e) {
  print('non-constructor:', e.constructor.name);
}
// CHECK: non-constructor: TypeError

class NoAllSettled extends Promise {
  static allSettled = null;
}
NoAllSettled.allSettledKeyed({}).then(() => {
  print('0) missing `allSettled` method does not throw, resolved');
}, e => {
  print('0) missing `allSettled` method does not throw, rejected', e.message);
});

class NoResolve extends Promise {
  static resolve = null;
}
NoResolve.allSettledKeyed({}).then(() => {
  print('1) NoResolve.allSettledKeyed: resolved');
}, e => {
  print('1) NoResolve.allSettledKeyed: rejected', e.message);
});

Promise.allSettledKeyed(function () {}).then((result) => {
  print('2) fn as argument: resolved', Reflect.ownKeys(result).length);
}, e => {
  print('2) fn as argument: rejected', e.message);
});

Promise.allSettledKeyed({}).then((result) => {
  print('3) Resolve with empty object, expected 0 keys:', Reflect.ownKeys(result).length);
});

const obj = { aaa: 111 };
Object.defineProperty(obj, 'bbb', { value: 222, enumerable: false });
Promise.allSettledKeyed(obj).then(result => {
  print('4) allSettledKeyed with non-enumerable property:', 'aaa' in result, 'bbb' in result);
});

const aa = Object.create({bb: 22}); // `bb` prop in prototype, not own prop
aa.aa = 11;
Promise.allSettledKeyed(aa).then(result => {
  print('5) allSettledKeyed own props, has "aa":', 'aa' in result, 'has "bb":', 'bb' in result);
});

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
  print('6) keys count:', Reflect.ownKeys(object).length);
  print('6) prototype:', Object.getPrototypeOf(object));
  print('6) a status/value:', object.a.status, object.a.value);
  print('6) b status/reason:', object.b.status, object.b.reason);
  print('6) nested status/value.c:', object.nested.status, object.nested.value.c instanceof Promise);
  print('6) symbol status/value:', object[symbol].status, object[symbol].value);
});
print('result instance:', result instanceof Promise);
// CHECK: result instance: true

// After microtask
// CHECK-NEXT: 0) missing `allSettled` method does not throw, resolved
// CHECK-NEXT: 1) NoResolve.allSettledKeyed: rejected Promise resolve is not a function
// CHECK-NEXT: 2) fn as argument: resolved 0
// CHECK-NEXT: 3) Resolve with empty object, expected 0 keys: 0
// CHECK-NEXT: 4) allSettledKeyed with non-enumerable property: true false
// CHECK-NEXT: 5) allSettledKeyed own props, has "aa": true has "bb": false
// CHECK-NEXT: 6) keys count: 4
// CHECK-NEXT: 6) prototype: null
// CHECK-NEXT: 6) a status/value: fulfilled 1
// CHECK-NEXT: 6) b status/reason: rejected 2
// CHECK-NEXT: 6) nested status/value.c: fulfilled true
// CHECK-NEXT: 6) symbol status/value: fulfilled true
