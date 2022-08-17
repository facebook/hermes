/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s

// Check that WeakMap doesn't crash due to a bug which caused it to write to
// internal valueStorage while it was being resized.
// This resulted in the free list being corrupted because it failed to retain
// state properly after a GC occurred.

var weak_map = new WeakMap();

for (var i = 0; i < 100000; ++i) {
  weak_map.set({}, i);
}
