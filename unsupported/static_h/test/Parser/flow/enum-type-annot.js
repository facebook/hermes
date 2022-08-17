/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

enum A : string {
  B,
}

// CHECK-LABEL: {{.*}}:10:8: error: '{' expected in enum declaration
// CHECK-NEXT:  enum A : string {
// CHECK-NEXT:  ~~~~~~~^
