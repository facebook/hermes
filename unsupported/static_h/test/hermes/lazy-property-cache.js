/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy -debug-only=codeblock -non-strict -target=HBC %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: debug_options, !fbcode

// lazy compilation leads to the property cache being allocated
// in a special fashion.

function foo(o) {
    o.p1 = "bar"
    print(o.p1)
    o.p1 = "nyan"
    print(o.p1)
}


var obj = {p1: "nyan"};

//CHECK-LABEL: lazy-prop-cache
print("lazy-prop-cache")

foo(obj)
//CHECK-NEXT: Compiling lazy function foo
//CHECK-NEXT: bar
//CHECK-NEXT: nyan

//Hitting the cache now
foo(obj)
gc()
//CHECK-NEXT: bar
//CHECK-NEXT: nyan

// CHECK-NEXT: end-lazy-prop-cache
print("end-lazy-prop-cache");
