/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type A = [...,];

// CHECK: {{.*}}:10:14: error: trailing commas after inexact tuple types are not allowed
// CHECK-NEXT: type A = [...,];
// CHECK-NEXT:              ^
