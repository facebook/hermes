// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

print("WeakMap");
// CHECK-LABEL: WeakMap
print(WeakMap.prototype)
// CHECK-NEXT: [object WeakMap]
print(WeakMap.length)
// CHECK-NEXT: 0
print(new WeakMap());
// CHECK-NEXT: [object WeakMap]
var x = {}
var y = {}
var m = new WeakMap(new Set([[x,2], [y,4]]));
print(m.get(x), m.get(y));
// CHECK-NEXT: 2 4

var iterable = {};
iterable[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: [{}, 2], done: false};
    },
    return: function() {
      print('returning');
    },
  };
};
var oldSet = WeakMap.prototype.set;
WeakMap.prototype.set = function() {
  throw new Error('add error');
}
try { new WeakMap(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
WeakMap.prototype.set = oldSet;
