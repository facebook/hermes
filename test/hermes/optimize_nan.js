/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:start
print("start");

// InstSimplify optimized this to true:
//CHECK-NEXT: false
print(0/0 == 0/0);

// In a different bug, IREval optimized this to true:
//CHECK-NEXT: false
print(undefined+undefined == undefined+1);

// Here's a generated list to exhaustively test various forms of NaN:
//  1. NaN is a runtime property lookup giving NaN
//  2. 0/0 which is (currently) a runtime calculation giving NaN
//  2. undefined+1 is a compile time calcululation giving NaN

//CHECK: false
print(NaN == NaN);
//CHECK: false
print(NaN == 0/0);
//CHECK: false
print(NaN == undefined+1);
//CHECK: false
print(0/0 == NaN);
//CHECK: false
print(0/0 == 0/0);
//CHECK: false
print(0/0 == undefined+1);
//CHECK: false
print(undefined+1 == NaN);
//CHECK: false
print(undefined+1 == 0/0);
//CHECK: false
print(undefined+1 == undefined+1);
//CHECK: false
print(NaN === NaN);
//CHECK: false
print(NaN === 0/0);
//CHECK: false
print(NaN === undefined+1);
//CHECK: false
print(0/0 === NaN);
//CHECK: false
print(0/0 === 0/0);
//CHECK: false
print(0/0 === undefined+1);
//CHECK: false
print(undefined+1 === NaN);
//CHECK: false
print(undefined+1 === 0/0);
//CHECK: false
print(undefined+1 === undefined+1);
//CHECK: false
print(NaN < NaN);
//CHECK: false
print(NaN < 0/0);
//CHECK: false
print(NaN < undefined+1);
//CHECK: false
print(0/0 < NaN);
//CHECK: false
print(0/0 < 0/0);
//CHECK: false
print(0/0 < undefined+1);
//CHECK: false
print(undefined+1 < NaN);
//CHECK: false
print(undefined+1 < 0/0);
//CHECK: false
print(undefined+1 < undefined+1);
//CHECK: false
print(NaN <= NaN);
//CHECK: false
print(NaN <= 0/0);
//CHECK: false
print(NaN <= undefined+1);
//CHECK: false
print(0/0 <= NaN);
//CHECK: false
print(0/0 <= 0/0);
//CHECK: false
print(0/0 <= undefined+1);
//CHECK: false
print(undefined+1 <= NaN);
//CHECK: false
print(undefined+1 <= 0/0);
//CHECK: false
print(undefined+1 <= undefined+1);
//CHECK: false
print(NaN > NaN);
//CHECK: false
print(NaN > 0/0);
//CHECK: false
print(NaN > undefined+1);
//CHECK: false
print(0/0 > NaN);
//CHECK: false
print(0/0 > 0/0);
//CHECK: false
print(0/0 > undefined+1);
//CHECK: false
print(undefined+1 > NaN);
//CHECK: false
print(undefined+1 > 0/0);
//CHECK: false
print(undefined+1 > undefined+1);
//CHECK: false
print(NaN >= NaN);
//CHECK: false
print(NaN >= 0/0);
//CHECK: false
print(NaN >= undefined+1);
//CHECK: false
print(0/0 >= NaN);
//CHECK: false
print(0/0 >= 0/0);
//CHECK: false
print(0/0 >= undefined+1);
//CHECK: false
print(undefined+1 >= NaN);
//CHECK: false
print(undefined+1 >= 0/0);
//CHECK: false
print(undefined+1 >= undefined+1);
//CHECK: true
print(NaN != NaN);
//CHECK: true
print(NaN != 0/0);
//CHECK: true
print(NaN != undefined+1);
//CHECK: true
print(0/0 != NaN);
//CHECK: true
print(0/0 != 0/0);
//CHECK: true
print(0/0 != undefined+1);
//CHECK: true
print(undefined+1 != NaN);
//CHECK: true
print(undefined+1 != 0/0);
//CHECK: true
print(undefined+1 != undefined+1);
//CHECK: true
print(NaN !== NaN);
//CHECK: true
print(NaN !== 0/0);
//CHECK: true
print(NaN !== undefined+1);
//CHECK: true
print(0/0 !== NaN);
//CHECK: true
print(0/0 !== 0/0);
//CHECK: true
print(0/0 !== undefined+1);
//CHECK: true
print(undefined+1 !== NaN);
//CHECK: true
print(undefined+1 !== 0/0);
//CHECK: true
print(undefined+1 !== undefined+1);
