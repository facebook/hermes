/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test transferring multiple ArrayBuffers in a single postMessage call.

var ab1 = new ArrayBuffer(2);
var ab2 = new ArrayBuffer(3);
var v1 = new Uint8Array(ab1);
v1.set([1, 2]);
var v2 = new Uint8Array(ab2);
v2.set([3, 4, 5]);

var worker = new Worker(`
  onmessage = function(msg) {
    var v1 = msg.v1;
    var v2 = msg.v2;
    postMessage("buf1: " + Array.prototype.join.call(v1, ","));
    postMessage("buf2: " + Array.prototype.join.call(v2, ","));
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
worker.postMessage({v1: v1, v2: v2}, [ab1, ab2]);

print("ab1 detached: " + (ab1.byteLength === 0));
print("ab2 detached: " + (ab2.byteLength === 0));

// CHECK: ab1 detached: true
// CHECK-NEXT: ab2 detached: true
// CHECK: buf1: 1,2
// CHECK-NEXT: buf2: 3,4,5
