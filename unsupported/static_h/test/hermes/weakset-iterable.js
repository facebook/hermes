/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

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
