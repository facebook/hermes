/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

// Symbols are a distinct primitive: arithmetic operations are not allowed.

(function main() {
  const s: symbol = Symbol.for('x');
  const n = s + 1;
//CHECK: {{.*}}symbol-error.js:14:13: error: ft: incompatible binary operation: + cannot be applied to symbol and 1
})();
