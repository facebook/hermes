/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

function main() {
  function f(a1) {
    let a1;
// CHECK: {{.*}}: error: Identifier 'a1' is already declared
  }
}
