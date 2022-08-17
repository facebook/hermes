/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1) | %FileCheck --match-full-lines %s

for([a, 0] of x);
//CHECK: {{.*}}for-of-in-destr.js:10:9: error: invalid assignment left-hand side
//CHECK-NEXT: for([a, 0] of x);
//CHECK-NEXT:         ^

for({a : 0, b} of x);
//CHECK: {{.*}}for-of-in-destr.js:15:10: error: invalid assignment left-hand side
//CHECK-NEXT: for({a : 0, b} of x);
//CHECK-NEXT:          ^

for([a, 0] in x);
//CHECK: {{.*}}for-of-in-destr.js:20:9: error: invalid assignment left-hand side
//CHECK-NEXT: for([a, 0] in x);
//CHECK-NEXT:         ^

for({a : 0, b} in x);
//CHECK: {{.*}}for-of-in-destr.js:25:10: error: invalid assignment left-hand side
//CHECK-NEXT: for({a : 0, b} in x);
//CHECK-NEXT:          ^
