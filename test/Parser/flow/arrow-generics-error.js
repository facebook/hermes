/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

<T>x => 42

// CHECK: {{.*}}:10:1: error: invalid expression {{.*}}
// CHECK-NEXT: <T>x => 42
// CHECK-NEXT: ^
