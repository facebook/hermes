/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

var x;
function f() {}
z;

// CHECK-LABEL: var x@global;
// CHECK-NEXT: function f@global() {}
// CHECK-NEXT: z@uglobal;
