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
// format and returns correct results.
var referenceNumberFormat = new Intl.NumberFormat("en", undefined);

// format() should not throw for BigInt inputs.
referenceNumberFormat.format(0n);
referenceNumberFormat.format(2n);
referenceNumberFormat.format(BigInt('2'));
referenceNumberFormat.format(BigInt('-42'));

// formatToParts() should not throw for BigInt inputs.
referenceNumberFormat.formatToParts(0n);
referenceNumberFormat.formatToParts(2n);
referenceNumberFormat.formatToParts(BigInt('2'));
referenceNumberFormat.formatToParts(BigInt('-42'));
