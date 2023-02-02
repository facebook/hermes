/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s

// This file tests IRGen in situations where it compiles the same AST more than once.
// It happens in two cases:
// - When compiling a loop pre-condition, it is compiled both as a pre and
//   post-condition.
// - When compiling a "finally" handler, it is inlined in the exception case
//   and the normal completion case.
//
// There are two important things to test in both of these cases above:
// - That the variables are not rebound again and the same variables are used.
// - That function expressions do not result in more than one IR Function.

function test_fexpr_in_while_cond() {
    while(function f(){}){
    }
}

function test_captured_fexpr_in_while() {
    while ((function fexpr() {
        if (cond) globalFunc = fexpr; 
        return something;
       })()) {
    }
}

function test_captured_fexpr_in_finally() {
    try {
        foo();
    } finally {
        (function fexpr(){
            if (!glob)
                glob = fexpr;
        })();
    }
}

function test_captured_let_in_finally() {
    try {
    } finally {
        glob = 0;
        let x;
        glob = function fexpr() { x = 10; }
    }
}

// Auto-generated content below. Please do not modify manually.
