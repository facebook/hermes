/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that a Worker can transfer an ArrayBuffer back to the main thread
// via postMessage with transfers.

var worker = new Worker(`
  onmessage = function(msg) {
    var ab = new ArrayBuffer(3);
    var view = new Uint8Array(ab);
    view[0] = 100;
    view[1] = 200;
    view[2] = msg;
    postMessage(view, [ab]);
  }
`);

worker.onmessage = function(msg) {
  var view = msg;
  print("byteLength: " + msg.byteLength);
  print("values: " + view[0] + "," + view[1] + "," + view[2]);
  worker.terminate();
}
worker.postMessage(42);

// CHECK: byteLength: 3
// CHECK-NEXT: values: 100,200,42
