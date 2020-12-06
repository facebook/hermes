/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-promise %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes -Xes6-promise %t.hbc | %FileCheck --match-full-lines %s

print('promise');
// CHECK-LABEL: promise

print(HermesInternal.hasPromise())
// CHECK-NEXT: true

var promise = new Promise(function(res, rej) {
  res('success!');
});

promise.then(function(message) {
  print('Resolved:', message);
});
// CHECK-NEXT: Resolved: success!

HermesInternal.enablePromiseRejectionTracker({
  allRejections: true,
  onUnhandled: function(id, error) {
    print("Unhandled:", error);
  },
});

var promise = new Promise(function(res, rej) {
  rej("failure!");
});

promise.then(function() {
  print('resolved');
});
// CHECK-NEXT: Unhandled: failure!
