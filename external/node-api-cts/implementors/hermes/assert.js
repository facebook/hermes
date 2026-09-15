/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Simple assert implementation for the Hermes CTS test harness.
// Injected as a global before each CTS test runs.
function assert(value, message) {
  if (!value) {
    throw new Error(message || 'Assertion failed');
  }
}
