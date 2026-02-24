/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

var anyArr: any[] = [];
([...anyArr]: number[]);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}array-literal-infer-error.js:11:3: error: ft: spread element with checked cast not supported
// CHECK-NEXT:([...anyArr]: number[]);
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
