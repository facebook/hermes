/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

({[x]=1}) => 0;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}object-binding-arrow-computed-error.js:10:6: error: ':' expected in property initialization
// CHECK-NEXT:({[x]=1}) => 0;
// CHECK-NEXT:  ~~~^
// CHECK-NEXT:Emitted 1 errors. exiting.
