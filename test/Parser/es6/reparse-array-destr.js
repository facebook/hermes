/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ir %s 2>&1 ) | %FileCheck --match-full-lines %s

[((a)), [(b)], c] = t;

([a, b]) = t;
//CHECK: {{.*}}reparse-array-destr.js:12:2: error: invalid assignment left-hand side
//CHECK-NEXT: ([a, b]) = t;
//CHECK-NEXT:  ^~~~~~

[(a = 1)] = t;
//CHECK: {{.*}}reparse-array-destr.js:17:3: error: invalid assignment left-hand side
//CHECK-NEXT: [(a = 1)] = t;
//CHECK-NEXT:   ^~~~~

[([b])] = t;
//CHECK: {{.*}}reparse-array-destr.js:22:3: error: invalid assignment left-hand side
//CHECK-NEXT: [([b])] = t;
//CHECK-NEXT:   ^~~
