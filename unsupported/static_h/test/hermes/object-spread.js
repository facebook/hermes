/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("BEGIN");
//CHECK: BEGIN

var t = {a:10, ..."he", ..."llo"};
print(Object.entries(t));
//CHECK-NEXT: 0,l,1,l,2,o,a,10


var t1 = {a1: 12, b: 21, b1: 22}
var t2 = {b1: 32, d:40}
var o1 = {a: 10, b:20, ...t1, c:30, ...t2};
print(Object.entries(o1));
//CHECK-NEXT: a,10,b,21,a1,12,b1,32,c,30,d,40

var t = {a:10, b:20};
var o = {...t, a:15};
print(Object.entries(o));
//CHECK-NEXT: a,15,b,20
