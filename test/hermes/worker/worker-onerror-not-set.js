/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that when onerror is not set, errors are silently discarded and the
// worker event loop continues processing subsequent messages.

var worker = new Worker(`
  onmessage = function(msg) {
    if (msg === "throw") {
      throw new Error("ignored error");
    }
    postMessage("got: " + msg);
  }
`);

// Intentionally do NOT set worker.onerror.

worker.onmessage = function(msg) {
  print(msg);
  worker.terminate();
}

// First message throws, but onerror is not set so error is discarded.
worker.postMessage("throw");
// Second message should still be processed.
worker.postMessage("hello");

// CHECK-NOT: ignored error
// CHECK: got: hello
