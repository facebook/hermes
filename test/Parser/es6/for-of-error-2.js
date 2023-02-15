/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

async function foo() { for (async = 1 of 2); }
// CHECK: {{.*}}:10:29: error: invalid assignment left-hand side
// CHECK-NEXT: async function foo() { for (async = 1 of 2); }
// CHECK-NEXT:                             ^~~~~~~~~
