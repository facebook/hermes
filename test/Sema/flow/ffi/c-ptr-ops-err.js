/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheck %s --match-full-lines

let _ = function (p: c_ptr): void {
    ++p;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: ft: update expression must be number or bigint
    p + 1;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: ft: incompatible binary operation: + cannot be applied to c_ptr and number
    p * 2;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: ft: incompatible binary operation: * cannot be applied to c_ptr and number
    p > p;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: ft: incompatible binary operation: > cannot be applied to c_ptr and c_ptr
    p !== "hello";
// FIXME! This should have been an error.
}

// CHECK: Emitted 4 errors. exiting.
