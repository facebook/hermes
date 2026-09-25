/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that calling close() inside a Worker stops processing future messages.
var worker = new Worker(`
  onmessage = function(msg) {
    if (msg == "close") {
      // Close the Worker.
      close();
    } else {
      postMessage("should not be sent");
    }
  }
`);

worker.onmessage = function(msg) {
  print(msg);
}

// Send a message to the Worker that will trigger it to close it.
worker.postMessage("close");
print("sent 'close' to worker");
// CHECK: sent 'close' to worker

// Send another message that should not be processed by the Worker.
worker.postMessage("lost message");
// CHECK-NOT: should not be sent
