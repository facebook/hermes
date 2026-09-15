/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

function foo(x: number, ...args: Array<string>): void {}

// CHECK: {{.*}}:13:1: error: ft: function expects at least 1 arguments, but 0 supplied
foo();

// CHECK: {{.*}}:16:8: error: ft: function parameter #2 type mismatch: cannot assign number to string
foo(1, 2);
