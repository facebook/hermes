/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that terminate stops the worker and no further messages are processed.

var worker = new Worker(`
  onmessage = function(msg) {
    postMessage("received: " + msg);
  }
`);

var messageCount = 0;
worker.onmessage = function(msg) {
  messageCount++;
  print(msg);
  // Terminate after the first message.
  worker.terminate();
  // This message should not be processed since the worker is terminated.
  worker.postMessage("second");
}
worker.postMessage("first");

// CHECK: received: first
// CHECK-NOT: received: second
