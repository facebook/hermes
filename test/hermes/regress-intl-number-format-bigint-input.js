/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s
// RUN: %hermes -O0 %s
// REQUIRES: intl

// Ensures Hermes Intl's NumberFormat doesn't crash on BigInt inputs to
// format.
var referenceNumberFormat = new Intl.NumberFormat("en", undefined);

try {
  referenceNumberFormat.format(0n);
} catch (err) {
}

try {
  referenceNumberFormat.formatToParts(0n);
} catch (err) {
}
