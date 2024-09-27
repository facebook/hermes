/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-transformed-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

async function f1(x = await 1) {}

async function f2(x = 1 + await 2) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}await-param-error.js:10:23: error: 'await' not allowed in a formal parameter
// CHECK-NEXT:async function f1(x = await 1) {}
// CHECK-NEXT:                      ^~~~~~~
// CHECK-NEXT:{{.*}}await-param-error.js:12:27: error: 'await' not allowed in a formal parameter
// CHECK-NEXT:async function f2(x = 1 + await 2) {}
// CHECK-NEXT:                          ^~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
