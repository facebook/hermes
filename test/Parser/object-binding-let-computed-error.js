/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

let { [x] } = 0;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-binding-let-computed-error.js:10:7: error: identifier expected in object binding pattern
// CHECK-NEXT:let { [x] } = 0;
// CHECK-NEXT:      ^
// CHECK-NEXT:Emitted 1 errors. exiting.
