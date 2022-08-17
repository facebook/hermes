/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that we don't allocate unbounded handles traversing the prototype chain.

var base = {}

var proto = {};
base.__proto__ = proto;

// Extend the prototype chain.
for (var i = 0; i < 1000; ++i) {
  var newProto = {};
  proto.__proto__ = newProto;
  proto = newProto;
}

// Use the [0] index to ensure that we use getComputed.
proto[0] = 3;

// Access property by forcing a loop through the chain.
print(base[0])
// CHECK: 3
