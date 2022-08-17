/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

var a = b: number => 42
// CHECK: {{.*}}:10:10: error: ';' expected
// CHECK-NEXT: var a = b: number => 42
// CHECK-NEXT:          ^
