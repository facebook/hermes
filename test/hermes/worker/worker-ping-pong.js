/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

var worker = new Worker(`
  onmessage = function(msg) {
    if (msg == "ping") {
      postMessage("pong");
    } else if (msg == "salt") {
      postMessage("pepper");
    }
  }
`);

worker.onmessage = function(msg) {
  if (msg == "pong") {
    print("received pong");
    worker.postMessage("salt");
  } else if (msg == "pepper") {
    print("received pepper");
    worker.terminate();
  }
}
worker.postMessage("ping");

// CHECK: received pong
// CHECK: received pepper
