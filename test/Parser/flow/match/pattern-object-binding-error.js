/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-match -dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

// Regression test for the same unchecked property value, reached through a
// binding pattern that is missing its identifier. The top-level form of this
// is covered by pattern-binding-error.js.

const e = match (x) { {a: const [y]}: 2 };

// CHECK: {{.*}}pattern-object-binding-error.js:14:33: error: 'identifier' expected in match binding pattern
// CHECK-NEXT: const e = match (x) { {a: const [y]}: 2 };
// CHECK-NEXT: {{.*}}~~~~~~^
// CHECK-NEXT: Emitted 1 errors. exiting.
