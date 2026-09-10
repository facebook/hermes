/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// Returning a thenable settles the result Promise through the Promise
// Resolution Procedure, which binds the thenable's `then`. A throw from that
// bind must reject the async function's Promise instead of leaving it pending
// forever. The trailing timer runs after the microtask queue drains, so it
// only prints last if the Promise really did settle.

var realBind = Function.prototype.bind;

async function returnsThenable() {
  return {
    then: function (resolve) {
      resolve('unreachable');
    },
  };
}

var resultPromise;
Function.prototype.bind = function () {
  throw new Error('hostile bind');
};
try {
  resultPromise = returnsThenable();
} finally {
  Function.prototype.bind = realBind;
}

resultPromise.then(
  function (value) {
    print('resolved', value);
  },
  function (error) {
    print('rejected', error.message);
  }
);

setTimeout(function () {
  print('drained');
}, 0);

// CHECK: rejected hostile bind
// CHECK-NEXT: drained
