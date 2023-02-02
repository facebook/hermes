/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-ir 2>&1 ) | %FileCheck --match-full-lines %s

// Due to restrictions in IRGen, we don't support assigning to
// "arguments" even in loose mode. This is not spec compliant!
function loose() {
    arguments = 0;
//CHECK: {{.*}}:[[@LINE-1]]:5: error: invalid assignment left-hand side
}

let arguments = 0;
function strict() {
    ++arguments;
//CHECK: {{.*}}:[[@LINE-1]]:7: error: invalid operand in update operation
}
