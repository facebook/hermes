/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// At most one indexer is allowed.
type TwoIndexers = {[string]: number, [number]: string};

// An indexer can't be combined with named properties.
type Mixed = {a: number, [string]: number};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-indexer-error.js:11:39: error: ft: at most one indexer is allowed in an object type
// CHECK-NEXT:type TwoIndexers = {[string]: number, [number]: string};
// CHECK-NEXT:                                      ^
// CHECK-NEXT:{{.*}}object-indexer-error.js:14:26: error: ft: indexers cannot be combined with named properties
// CHECK-NEXT:type Mixed = {a: number, [string]: number};
// CHECK-NEXT:                         ^
// CHECK-NEXT:Emitted 2 errors. exiting.
