/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -Xflow-parser -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

print("hello"+;)
//CHECK: {{.*}}simple-error-1.js:11:14: error: Unexpected token ;
//CHECK-NEXT: print("hello"+;)
//CHECK-NEXT:              ^
