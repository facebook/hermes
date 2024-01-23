/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck %s
// REQUIRES: es6_class

try {
  eval(new Uint16Array(33880).join(-1));
} catch (e) {
  print("Error in eval:", e.message);
}
// CHECK: Error in eval:
