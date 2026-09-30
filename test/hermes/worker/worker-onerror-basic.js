/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that worker.onerror is called when the worker's onmessage handler
// throws an error, and that the error value is correctly propagated.

var worker = new Worker(`
  onmessage = function(msg) {
    throw new Error("something went wrong");
  }
`);

worker.onerror = function(err) {
  print(err.message);
  worker.terminate();
}

worker.postMessage("trigger");

// CHECK: something went wrong
