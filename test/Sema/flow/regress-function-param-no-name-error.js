/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function f(cb: number => string) {
  cb('a');
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}regress-function-param-no-name-error.js:11:6: error: ft: function parameter #1 type mismatch
// CHECK-NEXT:  cb('a');
// CHECK-NEXT:     ^~~
// CHECK-NEXT:Emitted 1 errors. exiting.
