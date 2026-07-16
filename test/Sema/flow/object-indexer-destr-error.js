/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// A named property uses a string key, which can't index a number indexer.
function f1(x: {[number]: number}) {
  let {a, ...rest} = x;
}

// Reading through a writeonly indexer is not allowed.
function f2(x: {-[string]: number}) {
  let {a} = x;
}

// The same applies to the rest binding.
function f3(x: {-[string]: number}) {
  let {...rest} = x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-indexer-destr-error.js:14:8: error: ft: object index type string incompatible with index signature number
// CHECK-NEXT:  let {a, ...rest} = x;
// CHECK-NEXT:       ^
// CHECK-NEXT:{{.*}}object-indexer-destr-error.js:19:8: error: ft: cannot read writeonly indexer
// CHECK-NEXT:  let {a} = x;
// CHECK-NEXT:       ^
// CHECK-NEXT:{{.*}}object-indexer-destr-error.js:24:8: error: ft: cannot read writeonly indexer
// CHECK-NEXT:  let {...rest} = x;
// CHECK-NEXT:       ^~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
