/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec -typed %s | %FileCheck --match-full-lines %s

// Check that PrLoads generated in IRGen are not invalidated due to a
// reordering of object layout during later optimization passes.

var x: {1: void, a: string} = {
  1: undefined,
  a: "b",
};

print(x.a);
// CHECK: b

// Ensure x does not get promoted into the stack.
globalThis.escaping = x;
