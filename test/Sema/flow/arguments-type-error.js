/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals -typed -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

// Test that bare 'arguments' is an error in typed functions.
function typedBareArgs(x: number): void {
  // CHECK: {{.*}}:13:11: error: ft: 'arguments' is only allowed in 'arguments.length' in typed functions
  var a = arguments;
}

// Test that arguments.foo is an error in typed functions.
function typedArgsFoo(x: number): void {
  // CHECK: {{.*}}:19:11: error: ft: 'arguments' is only allowed in 'arguments.length' in typed functions
  var f = arguments.foo;
}

// Test that arguments[0] is an error in typed functions.
function typedArgsIndex(x: number): void {
  // CHECK: {{.*}}:25:11: error: ft: 'arguments' is only allowed in 'arguments.length' in typed functions
  var v = arguments[0];
}

// Test that arrows still inherit the typed outer 'arguments' restriction.
function typedOuterArrow(x: number): void {
  // CHECK: {{.*}}:31:17: error: ft: 'arguments' is only allowed in 'arguments.length' in typed functions
  var f = () => arguments[0];
}
