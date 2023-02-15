/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-ts -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

class A {
  private #foo = 1;
}
// CHECK-LABEL: {{.*}}:11:3: error: An accessibility modifier cannot be used with a private identifier
