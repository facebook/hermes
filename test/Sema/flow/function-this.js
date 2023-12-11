/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror --typed --dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

(function main() {

function f(this: number) {}
function g() {}
f = g;

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}function-this.js:14:1: error: ft: incompatible assignment types
// CHECK-NEXT:f = g;
// CHECK-NEXT:^~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
