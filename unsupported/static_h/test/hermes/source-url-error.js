/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Ensure that a custom sourceURL is used as the filename in stack traces.

print("START");
//CHECK: START

//# sourceURL=foo

try{ throw new Error(); } catch (e) {
    print(e.stack);
}
//CHECK-NEXT: Error
//CHECK-NEXT:   at global (foo:18:21)
