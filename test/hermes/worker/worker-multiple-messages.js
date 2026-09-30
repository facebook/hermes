/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that multiple messages can be sent to a Worker sequentially and
// the worker processes them in order.

var worker = new Worker(`
  var count = 0;
  onmessage = function(msg) {
    count++;
    postMessage("msg " + count + ": " + msg);
  }
`);

var received = 0;
worker.onmessage = function(msg) {
  received++;
  print(msg);
  if (received === 3) {
    worker.terminate();
  }
}

worker.postMessage("a");
worker.postMessage("b");
worker.postMessage("c");

// CHECK: msg 1: a
// CHECK-NEXT: msg 2: b
// CHECK-NEXT: msg 3: c
