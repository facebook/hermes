/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

enum E {
  A,
  ...,
  B,
}
// CHECK-LABEL: {{.*}}:12:3: error: The `...` must come after all enum members. Move it to the end of the enum body.
// CHECK-NEXT:   ...,
// CHECK-NEXT:   ^
