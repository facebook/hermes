/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("START");
//CHECK: START

function doit(a, ...rest) {
    print("a=", a, "rest=", rest);
}

doit();
//CHECK-NEXT: a= undefined rest=

doit(100);
//CHECK-NEXT: a= 100 rest=

doit(1, 2);
//CHECK-NEXT: a= 1 rest= 2

doit(1, 2, 3);
//CHECK-NEXT: a= 1 rest= 2,3
