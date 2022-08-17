/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheck %s

class C {
  foo(): number {}
}
// CHECK: {{.*}}:11:8: error: '{' expected in method definition
// CHECK-NEXT:   foo(): number {}
// CHECK-NEXT:   ~~~~~^
