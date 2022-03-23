/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -emit-binary -out=%t %s && %hermes -O -b %t | %FileCheck --match-full-lines %s
// Tests basic execution of bytecode (HBC).

print("bytecode");
//CHECK-LABEL: bytecode

e = 100;
try {
    throw Error();
} catch (e) {
    print(delete e);
//CHECK-NEXT: false
}
print(e);
//CHECK-NEXT: 100
