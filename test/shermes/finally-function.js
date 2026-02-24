/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function main(x) {
  try {
  } catch {
  } finally {
    return function nested() { print(x) };
  }
}

main(1)();
// CHECK: 1
