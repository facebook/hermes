/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that an ArrayBuffer can be transferred from the main thread to
// a Worker. After transfer, the original ArrayBuffer should be neutered
// (byteLength becomes 0).

var ab = new ArrayBuffer(4);
var view = new Uint8Array(ab);
view[0] = 10;
view[1] = 20;
view[2] = 30;
view[3] = 40;

print("before transfer: " + ab.byteLength);
// CHECK: before transfer: 4

var worker = new Worker(`
  onmessage = function(msg) {
    var view = msg;
    postMessage("length: " + msg.byteLength);
    postMessage("values: " + view[0] + "," + view[1] + "," + view[2] + "," + view[3]);
  }
`);

var received = 0;
worker.onmessage = function(msg) {
  received++;
  print(msg);
  if (received === 2) {
    worker.terminate();
  }
}
worker.postMessage(view, [ab]);

print("after transfer: " + ab.byteLength);
// CHECK-NEXT: after transfer: 0
// CHECK: length: 4
// CHECK-NEXT: values: 10,20,30,40
