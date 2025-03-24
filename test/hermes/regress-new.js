/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print("regress new");
// CHECK: regress new

// We correctly detect that constructing `this` in a construct call leaks its target closure, since it's accessible via `.constructor`.
(function () {
  function target({ a, b }) {
    return a + b;
  }

  function parent() {
    return new target({});
  }

  print(parent().constructor({ a: 10, b: 20 }));
// CHECK-NEXT: 30
})();
