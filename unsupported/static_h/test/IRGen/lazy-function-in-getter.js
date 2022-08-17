/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy %s
// Just make sure this function doesn't crash

function f() {
  return {
    get g() {
      function h() {
      }
    }
  };
}
print(f().g);
