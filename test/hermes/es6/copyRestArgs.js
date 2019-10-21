/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("START");
//CHECK: START

function doit(from) {
    print("from=", from, "rest=", HermesInternal.copyRestArgs(from));
}

doit(0);
//CHECK-NEXT: from= 0 rest= 0

doit(100);
//CHECK-NEXT: from= 100 rest=

doit("aaa");
//CHECK-NEXT: from= aaa rest= undefined

doit(0, 10, "a", "b");
//CHECK-NEXT: from= 0 rest= 0,10,a,b

doit(1, 10, "a", "b");
//CHECK-NEXT: from= 1 rest= 10,a,b

doit(3, 10, "a", "b");
//CHECK-NEXT: from= 3 rest= b

doit(-1, 10, "a", "b");
//CHECK-NEXT: from= -1 rest=
