/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

class C {
  +f() {}
}
// CHECK: {{.*}}:11:3: error: Unexpected variance sigil
// CHECK-NEXT:   +f() {}
// CHECK-NEXT:   ^
