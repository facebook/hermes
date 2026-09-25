/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that worker.onerror is called when the worker script throws during
// initial evaluation (before the event loop starts).

var worker = new Worker(`
  throw new Error("init failed");
`);

worker.onerror = function(err) {
  print("script error: " + err.message);
  worker.terminate();
}

// CHECK: script error: init failed
