/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that a Worker can be created and runs its script.
// The worker sends a message back to confirm it executed.

var worker = new Worker(`
  postMessage("hello from worker");
`);

worker.onmessage = function(msg) {
  print(msg);
  worker.terminate();
}

// CHECK: hello from worker
