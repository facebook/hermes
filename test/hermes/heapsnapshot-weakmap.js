/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Test that we are able to successfully create a heap snapshot from a WeakMap
// where some key objects have been freed.

print('Create heap snapshot');
// CHECK: Create heap snapshot
function f1() { }
var v1 = new Object();
new WeakMap([[{}, 1]]);
v1[f1(v1)], gc();
try {
  // We can't assert the output, just make sure it successfully finishes.
  createHeapSnapshot();
} catch (v10) {}
