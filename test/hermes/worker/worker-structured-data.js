/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck %s --match-full-lines

// Test that structured data (objects, arrays, numbers) can be sent
// between the main thread and the worker via postMessage.

var worker = new Worker(`
  onmessage = function(msg) {
    postMessage({
      name: msg.name,
      items: msg.items,
      count: msg.items.length,
    });
  }
`);

worker.onmessage = function(msg) {
  print("name: " + msg.name);
  print("count: " + msg.count);
  print("items: " + msg.items.join(", "));
  worker.terminate();
}
worker.postMessage({name: "test", items: [1, 2, 3]});

// CHECK: name: test
// CHECK-NEXT: count: 3
// CHECK-NEXT: items: 1, 2, 3
