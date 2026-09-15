/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-match -dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

// Regression test: a failure to parse the value of an object pattern property
// was not checked, so the null result was dereferenced right after the error
// was reported. Any pattern that fails to parse reaches this path.

const e = match (x) { {a: *}: 2 };

// CHECK: {{.*}}pattern-object-value-error.js:14:27: error: invalid match pattern
// CHECK-NEXT: const e = match (x) { {a: *}: 2 };
// CHECK-NEXT: {{.*}}^
// CHECK-NEXT: Emitted 1 errors. exiting.
