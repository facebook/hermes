/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-match -dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

// A match *statement* case body must be a block; only a match *expression*
// case body may be an arbitrary expression. `parseBlock` asserts that the
// current token is '{', so a non-block body here used to fail that assertion
// in a debug build instead of reporting an error. It must report instead.

match (x) { _ => 1 };

// CHECK: {{.*}}statement-non-block-body-error.js:15:18: error: '{' expected in 'match' statement case body
// CHECK-NEXT: match (x) { _ => 1 };
// CHECK-NEXT: {{.*}}~~~~~^
// CHECK-NEXT: Emitted 1 errors. exiting.
