/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// arguments doesn't exist in global scope. 
// v8 fails this test since it has a global property "arguments".

print(typeof arguments)
//CHECK: undefined

try {
    print(arguments);
} catch (e) {
    print(e);
}
//CHECK-NEXT: ReferenceError: Property 'arguments' doesn't exist
