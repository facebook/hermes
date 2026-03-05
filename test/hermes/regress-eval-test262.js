/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --test262 -Wno-direct-eval %s | %FileCheck --match-full-lines %s

// Verify that eval() picks up the --test262 flag: const reassignment
// should be deferred to runtime (TypeError) instead of rejected at
// compile time (SyntaxError).

try {
  eval("const x = 1; x = 2;");
} catch (e) {
  print(e.constructor.name);
}
// CHECK: TypeError
