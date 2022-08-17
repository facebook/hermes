/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

function *foo() { for (yield of x) print(1) }
// CHECK: {{.*}}:10:24: error: invalid assignment left-hand side
// CHECK: function *foo() { for (yield of x) print(1) }
// CHECK:                        ^~~~~
