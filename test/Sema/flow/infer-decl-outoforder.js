/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -fno-std-globals --typed --dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

'use strict';

(function(): void {

function foo(): void {
  z = 1;
}

// Doesn't infer the type because y,z is declared after x.
// Test just makes sure it doesn't crash.
let x = y + z;
let y = 0;
let z;

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}infer-decl-outoforder.js:20:9: error: local variable used prior to declaration
// CHECK-NEXT:let x = y + z;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}infer-decl-outoforder.js:20:13: error: local variable used prior to declaration
// CHECK-NEXT:let x = y + z;
// CHECK-NEXT:            ^
// CHECK-NEXT:Emitted 2 errors. exiting.
