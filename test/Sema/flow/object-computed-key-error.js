/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Mixing computed (indexer) and named properties is not supported.
function f1(k: string) {
  return {a: 1, [k]: 2};
}

// The computed value must satisfy the target indexer's value type.
function f2(k: string): {[string]: number} {
  return {[k]: "s"};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-computed-key-error.js:12:10: warning: ft: mixing computed and named properties in a typed object will result in an indexer
// CHECK-NEXT:  return {a: 1, [k]: 2};
// CHECK-NEXT:         ^~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}object-computed-key-error.js:17:16: error: ft: incompatible computed property value type
// CHECK-NEXT:  return {[k]: "s"};
// CHECK-NEXT:               ^~~
// CHECK-NEXT:Emitted 1 errors. exiting.
