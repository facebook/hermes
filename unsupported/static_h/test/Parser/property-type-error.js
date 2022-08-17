/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s -dump-ast 2>&1 ) | %FileCheck %s

({
  x(): number {}
})

// CHECK: {{.*}}:11:6: error: '{' expected in method definition
// CHECK-NEXT:   x(): number {}
// CHECK-NEXT:   ~~~^
