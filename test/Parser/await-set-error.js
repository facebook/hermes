/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

async function foo() {
  var x = {
    set y(val) {
      await new Promise();
    }
  }
}
// CHECK: {{.*}}:13:13: error: ';' expected
// CHECK:   await new Promise();
// CHECK:         ^
