/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s

// Promise.any errors must be in input order, not rejection order.
Promise.any([
  Promise.reject(1),
  Promise.resolve().then(() => Promise.reject(2)),
  Promise.reject(3),
]).catch(e => print(e.errors));
// CHECK: 1,2,3
