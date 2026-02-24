/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function do_minus(x) { return -x; }
function do_not(x) { return !x; }
function do_void(x) { return void x; }

print(do_minus(1));
// CHECK-LABEL: -1

print(do_not(1));
// CHECK-NEXT: false

print(do_void('asdf'));
// CHECK-NEXT: undefined
