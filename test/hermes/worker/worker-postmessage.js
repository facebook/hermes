/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that postMessage from main to worker works and the worker can
// echo the message back.

var worker = new Worker(`
  onmessage = function(msg) {
    postMessage("echo: " + msg);
  }
`);

worker.onmessage = function(msg) {
  print(msg);
  worker.terminate();
}
worker.postMessage("test message");

// CHECK: echo: test message
