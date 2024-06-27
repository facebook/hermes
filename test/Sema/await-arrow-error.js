/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

async (x = await) => {}

async (x = (await) => {}) => {}

async (x = (y = (await) => {}) => {}) => {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}await-arrow-error.js:10:12: error: await is not a valid identifier name in an async function
// CHECK-NEXT:async (x = await) => {}
// CHECK-NEXT:           ^~~~~
// CHECK-NEXT:{{.*}}await-arrow-error.js:12:13: error: await is not a valid identifier name in an async function
// CHECK-NEXT:async (x = (await) => {}) => {}
// CHECK-NEXT:            ^~~~~
// CHECK-NEXT:{{.*}}await-arrow-error.js:14:18: error: await is not a valid identifier name in an async function
// CHECK-NEXT:async (x = (y = (await) => {}) => {}) => {}
// CHECK-NEXT:                 ^~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
