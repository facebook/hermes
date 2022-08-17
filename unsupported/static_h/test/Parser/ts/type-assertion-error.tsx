/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-ts -parse-jsx -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

<T>x;
// CHECK-LABEL: {{.*}}:11:1: error: 'JSX text' expected in JSX child expression
// CHECK-NEXT: {{.*}}
// CHECK-NEXT: ^
