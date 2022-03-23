/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s

try {
  Array.from("1", RegExp);
} catch (e) {
  print (e);
}
// CHECK: SyntaxError: Invalid RegExp: Invalid flags
