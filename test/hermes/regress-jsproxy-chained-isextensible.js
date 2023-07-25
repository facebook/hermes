/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s

// Check for appropriate handle usage when chaining
// class to "isExtensible"
const proxy0 = new Proxy({}, {
  "isExtensible": () => { gc(); return true; }
});
const proxy1 = new Proxy(proxy0, {
  "isExtensible": Date
});
Object.isExtensible(proxy1);
