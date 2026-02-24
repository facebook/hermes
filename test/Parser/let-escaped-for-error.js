/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheckOrRegen %s

for (l\u0065t i of []) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}let-escaped-for-error.js:10:15: error: ';' or 'in' expected inside 'for'
// CHECK-NEXT:for (l\u0065t i of []) {}
// CHECK-NEXT:~~~~~~~~~~~~~~^
// CHECK-NEXT:Emitted 1 errors. exiting.
