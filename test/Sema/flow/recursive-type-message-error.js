/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Printing a recursive type without an alias name must terminate.

type A<T> = Array<A<T>>;
let x: A<number> = [];
let n: string = x[0];

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}recursive-type-message-error.js:14:5: error: ft: incompatible initialization type: cannot assign class Array<...> to string
// CHECK-NEXT:let n: string = x[0];
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
