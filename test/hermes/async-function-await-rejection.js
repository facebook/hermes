/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

function promiseWithThrowingConstructor(reason) {
  var promise = new Promise(function (resolve) {
    resolve();
  });
  Object.defineProperty(promise, 'constructor', {
    get: function () {
      throw reason;
    },
  });
  return promise;
}

async function testInitialPromiseResolveError() {
  try {
    await promiseWithThrowingConstructor('initial constructor getter');
  } catch (e) {
    print(e);
  }
}

async function returnValue(value) {
  return value;
}

async function throwBeforeAwait() {
  throw 'body throw before await';
}

async function test() {
  await testInitialPromiseResolveError();

  var returnedPromise = Promise.resolve('returned promise');
  var asyncResult = returnValue(returnedPromise);
  print('distinct result promise', asyncResult !== returnedPromise);
  print(await asyncResult);

  var rejectedResult;
  var threwSynchronously = false;
  try {
    rejectedResult = throwBeforeAwait();
  } catch (e) {
    threwSynchronously = true;
  }
  print('body throw was synchronous', threwSynchronously);
  try {
    await rejectedResult;
  } catch (e) {
    print(e);
  }

  print(await returnValue({
    then: function (resolve) {
      resolve('returned thenable');
    },
  }));

  try {
    await Promise.reject('promise rejection');
  } catch (e) {
    print(e);
  }

  print(await Promise.resolve('fulfillment'));

  try {
    await {
      then: function (_, reject) {
        reject('thenable rejection');
      },
    };
  } catch (e) {
    print(e);
  }

  try {
    await promiseWithThrowingConstructor('resumed constructor getter');
  } catch (e) {
    print(e);
  }

  var originalPromiseResolve = Promise.resolve;
  Promise.resolve = function () {
    throw 'overridden Promise.resolve';
  };
  try {
    print('primitive fulfillment', await 42);
  } finally {
    Promise.resolve = originalPromiseResolve;
  }

  print('thenable fulfillment', await {
    then: function (resolve) {
      resolve(43);
    },
  });

  throw 'body throw';
}

test().catch(function (e) {
  print(e);
});

// CHECK: initial constructor getter
// CHECK-NEXT: distinct result promise true
// CHECK-NEXT: returned promise
// CHECK-NEXT: body throw was synchronous false
// CHECK-NEXT: body throw before await
// CHECK-NEXT: returned thenable
// CHECK-NEXT: promise rejection
// CHECK-NEXT: fulfillment
// CHECK-NEXT: thenable rejection
// CHECK-NEXT: resumed constructor getter
// CHECK-NEXT: primitive fulfillment 42
// CHECK-NEXT: thenable fulfillment 43
// CHECK-NEXT: body throw
