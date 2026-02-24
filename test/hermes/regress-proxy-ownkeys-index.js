/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

// Regression test: JSProxy::ownPropertyKeys must convert string-valued
// array indices (e.g. "0") to numbers before comparing with targetKeys,
// because getOwnPropertyKeys stores indexed properties as numbers.
// Without the fix, the isSameValue invariant check fails with a TypeError
// for non-extensible targets that have indexed properties.

// Case 1: non-extensible plain object with numeric-string keys.
var obj = {"0": "a", "1": "b"};
Object.preventExtensions(obj);
var proxy1 = new Proxy(obj, {
  ownKeys: function(target) {
    return ["0", "1"];
  }
});
try {
  var keys1 = Object.getOwnPropertyNames(proxy1);
  print("keys1: " + keys1);
} catch (e) {
  print("keys1 error: " + e.constructor.name + ": " + e.message);
}
// CHECK: keys1: 0,1

// Case 2: non-extensible array (has indexed storage).
var arr = [10, 20, 30];
Object.preventExtensions(arr);
var proxy2 = new Proxy(arr, {
  ownKeys: function(target) {
    return ["0", "1", "2", "length"];
  }
});
try {
  var keys2 = Object.getOwnPropertyNames(proxy2);
  print("keys2: " + keys2);
} catch (e) {
  print("keys2 error: " + e.constructor.name + ": " + e.message);
}
// CHECK-NEXT: keys2: 0,1,2,length

// Case 3: sealed array (non-configurable + non-extensible).
var arr2 = [100, 200];
Object.seal(arr2);
var proxy3 = new Proxy(arr2, {
  ownKeys: function(target) {
    return ["0", "1", "length"];
  }
});
try {
  var keys3 = Object.getOwnPropertyNames(proxy3);
  print("keys3: " + keys3);
} catch (e) {
  print("keys3 error: " + e.constructor.name + ": " + e.message);
}
// CHECK-NEXT: keys3: 0,1,length

// Case 4: frozen object with numeric keys (non-configurable + non-writable).
var obj2 = {"0": "x", "1": "y", "foo": "bar"};
Object.freeze(obj2);
var proxy4 = new Proxy(obj2, {
  ownKeys: function(target) {
    return ["0", "1", "foo"];
  }
});
try {
  var keys4 = Object.getOwnPropertyNames(proxy4);
  print("keys4: " + keys4);
} catch (e) {
  print("keys4 error: " + e.constructor.name + ": " + e.message);
}
// CHECK-NEXT: keys4: 0,1,foo

// Case 5: Reflect.ownKeys on a non-extensible array (uses same code path).
var arr3 = ["a", "b"];
Object.preventExtensions(arr3);
var proxy5 = new Proxy(arr3, {
  ownKeys: function(target) {
    return Reflect.ownKeys(target);
  }
});
try {
  var keys5 = Reflect.ownKeys(proxy5);
  print("keys5: " + keys5);
} catch (e) {
  print("keys5 error: " + e.constructor.name + ": " + e.message);
}
// CHECK-NEXT: keys5: 0,1,length
