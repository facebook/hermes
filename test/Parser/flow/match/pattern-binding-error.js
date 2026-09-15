/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-match -dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

// Regression test: after reporting that a binding pattern is missing its
// identifier, the parser used to continue and read the identifier from the
// current (non-identifier) token, hitting an assert. It must stop parsing the
// binding pattern instead.

const e = match (x) { const [y]: 2 };

// CHECK: {{.*}}pattern-binding-error.js:15:29: error: 'identifier' expected in match binding pattern
// CHECK-NEXT: const e = match (x) { const [y]: 2 };
// CHECK-NEXT: {{.*}}~~~~~~^
// CHECK-NEXT: Emitted 1 errors. exiting.
