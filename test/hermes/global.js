/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -target=HBC %s | %FileCheck --match-full-lines %s
// Make sure the global object has a prototype and prints as the correct class.
// Also check that it can be accessed via globalThis.

print(this);
//CHECK: [object global]

print(globalThis);
//CHECK: [object global]

(function() {
print(globalThis);
//CHECK: [object global]
})();

var desc = Object.getOwnPropertyDescriptor(globalThis, 'globalThis');
print(desc.writable, desc.enumerable, desc.configurable);
//CHECK: true false true
