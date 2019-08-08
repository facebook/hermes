// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

print("Set prototype and constructor");
//CHECK-LABEL: Set prototype and constructor

print(Set.prototype);
//CHECK-NEXT: [object Set]
print(Set.length);
//CHECK-NEXT: 0

print(Object.getOwnPropertyNames(Set.prototype).sort(function(a,b) {
  if (a < b) return -1;
  if (a === b) return 0;
  if (a > b) return 1;
}));
//CHECK-NEXT: add,clear,constructor,delete,entries,forEach,has,keys,size,values
print(Set.prototype.keys === Set.prototype.values);
//CHECK-NEXT: true
print(Set.prototype.keys.name, Set.prototype.values.name);
//CHECK-NEXT: values values

print(new Set());
//CHECK-NEXT: [object Set]
var s = new Set(new Set([1, 2]));
print(s.has(1), s.has(2), s.has(3));
// CHECK-NEXT: true true false

try {
  Set();
} catch (e) {
  print(e);
}
//CHECK-NEXT: TypeError: Constructor Set requires 'new'

// Make sure we can insert internal object ID into a frozen object.
var fo = {};
Object.freeze(fo);
var fs = new Set();
fs.add(fo);

// Test some Set functions
var s = new Set();
print(s.size);
//CHECK-NEXT: 0
s.add(undefined);
s.add("abc");
print(s.size);
//CHECK-NEXT: 2
s = new Set([,10,20,,,30,10,,]);
print(s.size);
//CHECK-NEXT: 4
var it = s.values();
var t;
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: undefined false
t = it.next(); print(t.value, t.done);
//CHECK-NEXT: 10 false

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
var oldAdd = Set.prototype.add;
Set.prototype.add = function() {
  throw new Error('add error');
}
try { new Set(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
Set.prototype.add = oldAdd;
