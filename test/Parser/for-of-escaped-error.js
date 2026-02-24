/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

async function* f() {
  for await (var x o\u0066 []) ;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}for-of-escaped-error.js:11:20: error: ';' or 'in' expected inside 'for'
// CHECK-NEXT:  for await (var x o\u0066 []) ;
// CHECK-NEXT:  ~~~~~~~~~~~~~~~~~^
// CHECK-NEXT:Emitted 1 errors. exiting.
