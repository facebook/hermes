// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

print("WeakSet");
// CHECK-LABEL: WeakSet
print(WeakSet.prototype)
// CHECK-NEXT: [object WeakSet]
print(WeakSet.length)
// CHECK-NEXT: 0
print(new WeakSet());
// CHECK-NEXT: [object WeakSet]

var x = {}
var y = {}
var s = new WeakSet(new Set([x]));
print(s.has(x), s.has(y));
// CHECK-NEXT: true false

var iterable = {};
iterable[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: 1, done: false};
    },
    return: function() {
      print('returning');
    },
  };
};
var oldAdd = WeakSet.prototype.add;
WeakSet.prototype.add = function() {
  throw new Error('add error');
}
try { new WeakSet(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
WeakSet.prototype.add = oldAdd;
