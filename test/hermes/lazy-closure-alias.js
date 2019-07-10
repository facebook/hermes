// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -lazy -debug-only=codeblock -non-strict -target=HBC %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: debug_options

// Closure name should not shadow a local variable when lazily compiling that closure.

var f = function guard(){

var guard=42;

return function inner(){

print(typeof(guard));
/*
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 */
};

};
//CHECK-LABEL: lazy-closure-alias
print("lazy-closure-alias")

f()();
//CHECK-NEXT: Compiling lazy function guard
//CHECK-NEXT: Compiling lazy function inner
//CHECK-NEXT: number

// But if there's no collision, the closure name should be in scope.

var f2 = function guard2(){

var guard=42;

return function inner2(){

print(typeof(guard));
print(typeof(guard2));
/*
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 *  * This is just text to pass the lazy compilation threshold.
 */
};

};

f2()();
//CHECK-NEXT: Compiling lazy function guard2
//CHECK-NEXT: Compiling lazy function inner2
//CHECK-NEXT: number
//CHECK-NEXT: function

// CHECK-NEXT: end-lazy-closure-alias
print("end-lazy-closure-alias");
