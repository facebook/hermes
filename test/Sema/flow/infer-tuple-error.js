/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

([1, 2, 3]: [number]);
([1]: [number, number]);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}infer-tuple-error.js:12:2: error: ft: incompatible tuple type, expected 1 elements, found 3
// CHECK-NEXT:([1, 2, 3]: [number]);
// CHECK-NEXT: ^~~~~~~~~
// CHECK-NEXT:{{.*}}infer-tuple-error.js:13:2: error: ft: incompatible tuple type, expected 2 elements, found 1
// CHECK-NEXT:([1]: [number, number]);
// CHECK-NEXT: ^~~
// CHECK-NEXT:Emitted 2 errors. exiting.
