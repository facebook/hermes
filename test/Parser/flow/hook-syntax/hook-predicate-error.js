/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

hook useFoo1(): %checks => number;
// CHECK: {{.*}}:10:17: error: checks predicates unsupported with hooks
// CHECK-NEXT: hook useFoo1(): %checks => number;
// CHECK-NEXT:                 ^
