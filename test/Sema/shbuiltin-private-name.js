/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

// Regression test: calling a private member of $SHBuiltin used to crash the
// resolver, which cast the member expression's property to an identifier
// without checking for a PrivateNameNode. A private property is never a
// builtin access, so this must be reported like any other invalid use of
// $SHBuiltin.

class C {
  #x;
  m() {
    $SHBuiltin.#x();
  }
}
// CHECK: {{.*}}shbuiltin-private-name.js:[[@LINE-3]]:5: error: invalid use of $SHBuiltin
