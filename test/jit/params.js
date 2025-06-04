/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xjit=force -Xdump-jitcode=2 %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo(a, b) {
    return a + b;
}

function bar(
    a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
    a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32) {
    return a1 + a32;
}

print(foo("10"));
// CHECK: JIT successfully compiled FunctionID 1, 'foo'
// CHECK-NEXT: 10undefined

print(bar("1+", 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32, 33));
// CHECK: JIT successfully compiled FunctionID 2, 'bar'
// CHECK-NEXT: 1+32
