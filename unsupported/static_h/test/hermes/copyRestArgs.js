/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

print("START");
//CHECK: START

function doit(from) {
    print("from=", from, "rest=", HermesInternal.copyRestArgs(from));
}

(function(...rest) { print("from=", 0, "rest=", rest); })(0);
//CHECK-NEXT: from= 0 rest= 0

(function(a, b, c, d, ...rest) { print("from=", 5, "rest=", rest); })(0);
//CHECK-NEXT: from= 5 rest=

(function(...rest) { print("from=", 0, "rest=", rest); })(0, 10, "a", "b");
//CHECK-NEXT: from= 0 rest= 0,10,a,b

(function(a, ...rest) { print("from=", 1, "rest=", rest); })(0, 10, "a", "b");
//CHECK-NEXT: from= 1 rest= 10,a,b

(function(a, b, c, ...rest) { print("from=", 3, "rest=", rest); })(3, 10, "a", "b");
//CHECK-NEXT: from= 3 rest= b
