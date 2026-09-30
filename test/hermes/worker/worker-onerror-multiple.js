/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that multiple errors from a worker are all dispatched to onerror.

var worker = new Worker(`
  onmessage = function(msg) {
    if (msg === "ok") {
      postMessage("done");
      return;
    }
    throw new Error("error: " + msg);
  }
`);

var errorCount = 0;

worker.onerror = function(err) {
  errorCount++;
  print("onerror " + errorCount + ": " + err.message);
  if (errorCount < 3) {
    // Send another message that will throw.
    worker.postMessage("fail" + (errorCount + 1));
  } else {
    // After 3 errors, send a message that succeeds.
    worker.postMessage("ok");
  }
}

worker.onmessage = function(msg) {
  print(msg);
  print("total errors: " + errorCount);
  worker.terminate();
}

worker.postMessage("fail1");

// CHECK: onerror 1: error: fail1
// CHECK-NEXT: onerror 2: error: fail2
// CHECK-NEXT: onerror 3: error: fail3
// CHECK-NEXT: done
// CHECK-NEXT: total errors: 3
