/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that the worker event loop continues after an error in onmessage.
// The worker should be able to process subsequent messages after throwing.

var worker = new Worker(`
  onmessage = function(msg) {
    if (msg === "throw") {
      throw new Error("handled error");
    }
    postMessage("processed: " + msg);
  }
`);

var errorCount = 0;
var messageCount = 0;

worker.onerror = function(err) {
  errorCount++;
  print("error " + errorCount + ": " + err.message);
  // Send another message after the error to verify the worker is still alive.
  worker.postMessage("after-error");
}

worker.onmessage = function(msg) {
  messageCount++;
  print("message " + messageCount + ": " + msg);
  worker.terminate();
}

worker.postMessage("throw");

// CHECK: error 1: handled error
// CHECK-NEXT: message 1: processed: after-error
