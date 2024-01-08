/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-lra -O %s | %FileCheckOrRegen %s --match-full-lines

// Positive zero is 'cheap'.
function poszero(f) {
  return f(0.0, 0.0);
}

// Negative zero is NOT 'cheap'.
function negzero(f) {
  return f(-0.0, -0.0);
}
