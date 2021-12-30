/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print("BEGIN");
//CHECK: BEGIN

var a = [1, 2, 3];
var [b, ...c] = a;
print(b, c);
//CHECK-NEXT: 1 2,3

var [_, ...d] = "hello";
print(d);
//CHECK-NEXT: e,l,l,o

var e = [1, 2, [3, 4, 5]];
var [_, _, [_, ...f]] = e;
print(f);
//CHECK-NEXT: 4,5
