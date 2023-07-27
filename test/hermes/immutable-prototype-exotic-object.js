/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Setting the same value is valid.
({}).__proto__.__proto__ = ({}).__proto__.__proto__;

try {
  ({}).__proto__.__proto__ = {};
} catch (e) {
  print(e.name, e.message);
}
// CHECK: TypeError Cannot set prototype of immutable prototype object
