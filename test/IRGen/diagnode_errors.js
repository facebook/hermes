// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermes -strict -dump-ir %s) 2>&1 | %FileCheck %s --match-full-lines

function one() { return s; return s; }
//CHECK: {{.*}}diagnode_errors.js:8:25: warning: the variable "s" was not declared in function "one"
//CHECK-NEXT: function one() { return s; return s; }
//CHECK-NEXT:                         ^

function two() { return s; return t;}
//CHECK: {{.*}}diagnode_errors.js:13:35: warning: the variable "t" was not declared in function "two"
//CHECK-NEXT: function two() { return s; return t;}
//CHECK-NEXT:                                   ^

function three() { return z; return z;}
//CHECK: {{.*}}diagnode_errors.js:18:27: warning: the variable "z" was not declared in function "three"
//CHECK-NEXT: function three() { return z; return z;}
//CHECK-NEXT:                           ^

function four() { with({}) {}; }
//CHECK: {{.*}}diagnode_errors.js:23:19: error: invalid statement encountered.
//CHECK-NEXT: function four() { with({}) {}; }
//CHECK-NEXT:                   ^~~~~~~~~~~


//CHECK:{{.*}}warning: the property "color" was set multiple times in the object definition.
//CHECK-NEXT:var x = { color: 10, color: 20 };
//CHECK-NEXT:                            ^~
//CHECK-NEXT:{{.*}} warning: The first definition was here.
//CHECK-NEXT:var x = { color: 10, color: 20 };
//CHECK-NEXT:          ^~~~~~~~~
var x = { color: 10, color: 20 };

//CHECK: Emitted 1 errors. exiting.
