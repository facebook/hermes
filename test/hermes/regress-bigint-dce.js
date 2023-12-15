/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

(function() {

// Verify that arithmetic operations with a BigInt operand and non-BigInt
// operand (except adds) are not DCE-d, because that would throw a runtime
// exception.

try {
    1n + 1;
} catch (e) {
    print(e);
}
//CHECK: TypeError: Cannot convert 1 to BigInt

try {
    1 << 1n;
} catch (e) {
    print(e);
}
//CHECK-NEXT: TypeError: Cannot convert BigInt to number

// Verify that BigInt operations that may throw are not eliminated:
// unsigned right shift, division, modulo.
try {
    1n >>> 1n;
} catch (e) {
    print(e);
}
//CHECK-NEXT: TypeError: BigInts have no unsigned shift

try {
    1n / 0n;
} catch (e) {
    print(e);
}
//CHECK-NEXT: RangeError: Division by zero

try {
    1n % 0n;
} catch (e) {
    print(e);
}
//CHECK-NEXT: RangeError: Division by zero

})();
