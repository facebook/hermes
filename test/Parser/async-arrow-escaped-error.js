/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

\u0061sync () => {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}async-arrow-escaped-error.js:10:1: error: invalid arrow function parameter list
// CHECK-NEXT:\u0061sync () => {}
// CHECK-NEXT:^~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
