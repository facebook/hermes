/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Spreading a writeonly indexer reads it, which is not allowed.
function f1(o: {-[string]: number}) {
  let x = {...o};
}

// A named property uses a string key, which cannot index a number-keyed
// indexer that came from a spread source.
function f2(o: {[number]: number}) {
  let x = {...o, a: 1};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-indexer-spread-error.js:12:12: error: ft: cannot read writeonly indexer
// CHECK-NEXT:  let x = {...o};
// CHECK-NEXT:           ^~~~
// CHECK-NEXT:{{.*}}object-indexer-spread-error.js:18:11: error: ft: object index type string incompatible with index signature number
// CHECK-NEXT:  let x = {...o, a: 1};
// CHECK-NEXT:          ^~~~~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
