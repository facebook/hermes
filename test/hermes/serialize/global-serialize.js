// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

// Test the global object
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
