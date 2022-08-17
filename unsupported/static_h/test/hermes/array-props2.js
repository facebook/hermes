/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

x = Array(5);
print(x);
//CHECK: ,,,,

p = x.__proto__;
Object.defineProperty(p, "7", {get: function() { return 10; }, set: function (v) {
    print("set1", v); }});
Object.defineProperty(p, "2", {get: function() { return 9; }, set: function (v) {
    print("set2", v); }});

x[2] = 100;
//CHECK-NEXT: set2 100
x[7] = 101;
//CHECK-NEXT: set1 101
print(x[0], x[2], x[7], x.length);
//CHECK-NEXT: undefined 9 10 5
