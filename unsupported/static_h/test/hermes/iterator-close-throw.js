/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print('iterator close/return');
// CHECK-LABEL: iterator close/return

// GetMethod errors should be suppressed if there's already an error completion.
var iter = {
  [Symbol.iterator]() {
    return this;
  },
  next() {
    return {value: 1, done: false};
  },
  return: 0,
};

try {
  // Test iteration instructions.
  for (var i of iter) {
    throw 1;
  }
} catch (e) {
  print('caught', e);
}
// CHECK: caught 1

try {
  // Test slow iteration instructions.
  function* gen() {
    yield* iter;
  }
  var it = gen();
  it.throw(1);
} catch (e) {
  print('caught', e);
}
// CHECK: caught 1

try {
  // Test JS lib internal iteration logic.
  Array.from(iter, function() {
    throw 1;
  });
} catch (e) {
  print('caught', e);
}
// CHECK: caught 1

// GetMethod errors are not suppressed if the break was not a throw.
var iter = {
  [Symbol.iterator]() {
    return this;
  },
  next() {
    return {value: 1, done: false};
  },
  get return() {
    throw 0;
  }
};

try {
  for (var i of iter) {
    break;
  }
} catch (e) {
  print('caught', e);
}
// CHECK: caught 0
