/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

declare var +;

// CHECK-LABEL: {{.*}}:10:13: error: 'identifier' expected in var declaration
// CHECK-NEXT: declare var +;
// CHECK-NEXT: ~~~~~~~~~~~~^
