/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

(function() {

function foo(x: number|string): number { return 2; }
function bar(x: number): number|string { return x; }

// Ensure foo flows into bar.
bar = foo;

print(bar(1));
// CHECK: 2

})();
