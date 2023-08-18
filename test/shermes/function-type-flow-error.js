/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -exec %s 2>&1) | %FileCheck --match-full-lines %s

(function(): void {

function foo(x: number|string): number { return 2; }
function bar(x: number): number|string { return x; }

// Ensure bar cannot flow into foo.
foo = bar;
// CHECK-LABEL: {{.*}}:16:1: error: ft: incompatible assignment types
// CHECK-NEXT: foo = bar;
// CHECK-NEXT: ^~~~~~~~~

})();
