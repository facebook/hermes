/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test the 'symbol' primitive type and the Symbol.for static method.

'use strict';

(function () {
  // Symbol.for returns a 'symbol'.
  const s: symbol = Symbol.for('x');
  print(typeof s);
  // CHECK: symbol

  // Registered symbols with the same key are identical.
  print(Symbol.for('x') === s);
  // CHECK-NEXT: true

  // Different keys produce different symbols.
  print(Symbol.for('x') === Symbol.for('y'));
  // CHECK-NEXT: false

  // A symbol flows into a symbol-typed binding.
  const t: symbol = s;
  print(t === s);
  // CHECK-NEXT: true

  // Symbols can be passed as typed parameters.
  function eq(a: symbol, b: symbol): boolean {
    return a === b;
  }
  print(eq(s, Symbol.for('x')));
  // CHECK-NEXT: true
  print(eq(s, Symbol.for('z')));
  // CHECK-NEXT: false
})();
