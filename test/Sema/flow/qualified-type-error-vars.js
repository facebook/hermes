/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// We don't support qualified type annotations yet.
// These tests are just to ensure shermes doesn't crash.
// They can be removed once we support qualified type annotations.

var a: A.B;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}qualified-type-error-vars.js:14:8: error: ft: unsupported type annotation
// CHECK-NEXT:var a: A.B;
// CHECK-NEXT:       ^~~
// CHECK-NEXT:Emitted 1 errors. exiting.
