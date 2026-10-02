
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
// CHECK: true
print('typeof Promise.allKeyed:', typeof Promise.allKeyed);
// CHECK: typeof Promise.allKeyed: function



// Subclass dispatch: returned promise is an instance of the subclass.
class MyPromise extends Promise {}
var wrSub = MyPromise.allKeyed({});
print('subclass instance:', wrSub instanceof MyPromise);
// CHECK: subclass instance: true
var wrSubCall = Promise.allKeyed.call(MyPromise, {});
print('subclass call instance:', wrSubCall instanceof MyPromise);
// CHECK: subclass call instance: true

try {
  Promise.allKeyed.call(undefined, {});
  print('non-constructor: no throw');
} catch (e) {
  print('non-constructor:', e.constructor.name);
}
// CHECK: non-constructor: TypeError


class NoAll extends Promise {
  static all = null;
}
NoAll.allKeyed({}).then(() => {
  print('0) missing `all` method does not throw, resolved');
}, e => {
  print('0) missing `all` method does not throw, rejected', e.message);
});

class NoResolve extends Promise {
  static resolve = null;
}
NoResolve.allKeyed({}).then(() => {
  print('1) NoResolve.allKeyed: resolved');
}, e => {
  print('1) NoResolve.allKeyed: rejected', e.message);
});

Promise.allKeyed(function () {}).then((result) => {
  print('2) fn as argument: resolved',  Reflect.ownKeys(result).length);
}, e => {
  print('2) fn as argument: rejected', e.message);
});

Promise.allKeyed({}).then((result) => {
  print('3) Resolve with empty object, expected 0 keys:', Reflect.ownKeys(result).length);
});

const obj = { aaa: 111 };
Object.defineProperty(obj, 'bbb', { value: 222, enumerable: false });
Promise.allKeyed(obj).then(result => {
  print('4) allKeyed with non-enumerable property:', 'aaa' in result, 'bbb' in result);
});

const aa = Object.create({bb: 22});  // `bb` prop in prototype, not own prop
aa.aa = 11;
Promise.allKeyed(aa).then(result => {
  print('5) allKeyed own props, has "aa":', 'aa' in result, 'has "bb":', 'bb' in result);
});


// Verify shape of the returned object.
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
  print('6) keys count:', Reflect.ownKeys(object).length);
  print('6) prototype:', Object.getPrototypeOf(object));
  print('6) typeof a:', typeof object.a);
  print('6) typeof b:', typeof object.b);
  print('6) typeof nexted.c:', object.nexted.c instanceof Promise);
  print('6) typeof symbol:', typeof object[symbol]);
});
print('result instance:', result instanceof Promise);
// CHECK: result instance: true


// After microtask

// CHECK-NEXT: 0) missing `all` method does not throw, resolved
// CHECK-NEXT: 1) NoResolve.allKeyed: rejected Promise resolve is not a function
// CHECK-NEXT: 2) fn as argument: resolved 0
// CHECK-NEXT: 3) Resolve with empty object, expected 0 keys: 0
// CHECK-NEXT: 4) allKeyed with non-enumerable property: true false
// CHECK-NEXT: 5) allKeyed own props, has "aa": true has "bb": false
// CHECK-NEXT: 6) keys count: 4
// CHECK-NEXT: 6) prototype: null
// CHECK-NEXT: 6) typeof a: number
// CHECK-NEXT: 6) typeof b: string
// CHECK-NEXT: 6) typeof nexted.c: true
// CHECK-NEXT: 6) typeof symbol: boolean
