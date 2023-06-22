/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -fno-static-builtins %s | %FileCheckOrRegen --match-full-lines %s

this.__defineGetter__("Object", () => 1.1);
try {
  eval(`
      function __f_0() {
        "use static builtin";
      }
  `);
} catch (e) {
  print(e.name, e.message);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:TypeError Cannot execute a bytecode compiled with -fstatic-builtins: Object is not an object
