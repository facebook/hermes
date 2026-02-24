/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

let func = $SHBuiltin.extern_c({}, function func(x: c_int): void {});

func();
// CHECK: {{.*}}:[[@LINE-1]]:1: error: ft: function expects 1 arguments, but 0 supplied

func("foo");
// CHECK: {{.*}}:[[@LINE-1]]:6: error: ft: function parameter 'x' type mismatch
