/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s

// Test that a Worker created inside a local scope (function) is properly
// cleaned up by the engine when the variable goes out of scope. The worker
// sends multiple messages to the main thread. After the function returns,
// the only reference to the Worker is through the onmessage closure.

function createLocalWorker() {
  var worker = new Worker(`
    postMessage("local msg 1");
    postMessage("local msg 2");
    postMessage("local msg 3");
  `);

  var received = 0;
  worker.onmessage = function(msg) {
    print(msg);
  }
}

createLocalWorker();
gc();
