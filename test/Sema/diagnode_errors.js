/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -strict -dump-ir %s 2>&1 | %FileCheck %s --match-full-lines

function one() { return s; return s; }
//CHECK: {{.*}}diagnode_errors.js:10:25: warning: the variable "s" was not declared in function "one"
//CHECK-NEXT: function one() { return s; return s; }
//CHECK-NEXT:                         ^

function two() { return s; return t;}
//CHECK: {{.*}}diagnode_errors.js:15:35: warning: the variable "t" was not declared in function "two"
//CHECK-NEXT: function two() { return s; return t;}
//CHECK-NEXT:                                   ^

function three() { return z; return z;}
//CHECK: {{.*}}diagnode_errors.js:20:27: warning: the variable "z" was not declared in function "three"
//CHECK-NEXT: function three() { return z; return z;}
//CHECK-NEXT:                           ^

(function () { return inAnonymous; })()
//CHECK: {{.*}}warning: the variable "inAnonymous" was not declared in anonymous function
//CHECK-NEXT: (function () { return inAnonymous; })()
//CHECK-NEXT:                       ^~~~~~~~~~~

(() => { return inAnonymousArrow; })()
//CHECK: {{.*}}warning: the variable "inAnonymousArrow" was not declared in anonymous arrow function
//CHECK-NEXT: (() => { return inAnonymousArrow; })()
//CHECK-NEXT:                 ^~~~~~~~~~~~~~~~

var inferredName = () => { return i }; inferredName();
//CHECK: {{.*}}warning: the variable "i" was not declared in anonymous arrow function
//CHECK-NEXT: var inferredName = () => { return i }; inferredName();
//CHECK-NEXT:                                   ^
