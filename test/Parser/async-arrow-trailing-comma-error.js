/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

(async (...a,) => { });

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}async-arrow-trailing-comma-error.js:10:13: error: Rest parameter must be last formal parameter
// CHECK-NEXT:(async (...a,) => { });
// CHECK-NEXT:            ^
// CHECK-NEXT:Emitted 1 errors. exiting.
