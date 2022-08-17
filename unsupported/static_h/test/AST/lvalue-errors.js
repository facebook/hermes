/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -hermes-parser %s 2>&1 || true) | %FileCheck %s --match-full-lines

var x;
x + 1 = 10;
//CHECK: {{.*}}lvalue-errors.js:11:1: error: invalid assignment left-hand side
//CHECK-NEXT: x + 1 = 10;
//CHECK-NEXT: ^~~~~

for (x + 1 in x);
//CHECK: {{.*}}lvalue-errors.js:16:6: error: invalid assignment left-hand side
//CHECK-NEXT: for (x + 1 in x);
//CHECK-NEXT:      ^~~~~

for (x + 1 of x);
//CHECK: {{.*}}lvalue-errors.js:21:6: error: invalid assignment left-hand side
//CHECK-NEXT: for (x + 1 of x);
//CHECK-NEXT:      ^~~~~

++0;
//CHECK: {{.*}}lvalue-errors.js:26:3: error: invalid operand in update operation
//CHECK-NEXT: ++0;
//CHECK-NEXT:   ^
