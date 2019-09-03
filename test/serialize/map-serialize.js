// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-after-init-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

print("Map prototype and constructor");
//CHECK-LABEL: Map prototype and constructor

print(Map.prototype);
//CHECK-NEXT: [object Map]
print(Map.length);
//CHECK-NEXT: 0

print(Object.getOwnPropertyNames(Map.prototype));
//CHECK-NEXT: clear,delete,entries,forEach,get,has,keys,set,size,values,constructor

print(new Map());
//CHECK-NEXT: [object Map]
var x = new Map(new Set([[1,2], [3,4]]));
print(x.get(1), x.get(3));
// CHECK-NEXT: 2 4

try {
  Map();
} catch (e) {
  print(e);
}
//CHECK-NEXT: TypeError: Constructor Map requires 'new'

var iterable = {};
iterable[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: [1, 2], done: false};
    },
    return: function() {
      print('returning');
    },
  };
};
var oldSet = Map.prototype.set;
Map.prototype.set = function() {
  throw new Error('add error');
}
try { new Map(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
Map.prototype.set = oldSet;
