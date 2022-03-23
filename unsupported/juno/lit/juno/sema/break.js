/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-sema

// Test to ensure none of these cause errors.

while (1) {
  break;
}

while (1) {
  switch (1) {
    default:
      break;
  }
}

switch (1) {
  default:
  while (1) {
    break;
  }
}

switch (1) {
  default:
  break;
}
