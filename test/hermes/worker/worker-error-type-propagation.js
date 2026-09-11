/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that different error types and non-Error thrown values are correctly
// propagated to onerror.

var worker = new Worker(`
  onmessage = function(msg) {
    if (msg == 0) {
      throw new RangeError("out of range");
    } else if (msg == 1) {
      throw "a string error";
    } else if (msg == 2) {
      close();
    }
  }
`);

var step = 0;

worker.onerror = function(err) {
  step++;
  if (step == 1) {
    // RangeError
    print("step 1: " + err.message);
    worker.postMessage(1);
  } else if (step == 2) {
    // String throw
    print("step 2: " + err);
    worker.postMessage("2");
  }
}

worker.postMessage(step);

// CHECK: step 1: out of range
// CHECK-NEXT: step 2: a string error
