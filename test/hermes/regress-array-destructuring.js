/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var iterator = {
  next: function() {
    return {done: false, value: undefined};
  },
  return: function() {
    print("iterator closed");
    return {done: false, value: undefined};
  }
};
var iterable = {};
iterable[Symbol.iterator] = function() {
  return iterator;
};

function *myGen(o) {
  [(yield).p] = o;
}
let genObj = myGen(iterable);
genObj.next();
genObj.return();

// CHECK: iterator closed
