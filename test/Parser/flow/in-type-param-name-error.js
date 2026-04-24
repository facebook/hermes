/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

// Numeric literal in type-parameter position is not a valid name.
// Exercises the "no name token, no deferred `in`" error branch.
type T = <0>(x: number) => number;
// CHECK: {{.*}}:12:11: error: 'identifier' expected in type parameter
// CHECK-NEXT: type T = <0>(x: number) => number;
// CHECK-NEXT:           ^
