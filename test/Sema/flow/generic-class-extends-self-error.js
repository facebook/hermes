/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class GenSelf<T> extends GenSelf<T> {}
new GenSelf<number>();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-class-extends-self-error.js:10:26: error: ft: super class used before it is defined
// CHECK-NEXT:class GenSelf<T> extends GenSelf<T> {}
// CHECK-NEXT:                         ^
// CHECK-NEXT:Emitted 1 errors. exiting.
