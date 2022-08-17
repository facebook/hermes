/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O0 -commonjs %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -commonjs -fstatic-require -fstatic-builtins %S/ -emit-binary -out %T/test.hbc && %hermes %T/test.hbc | %FileCheck --match-full-lines %s

// Tests that loading any copy of a repeated CommonJS module works the same.

// Load segment 1, then segment 2 (the same order as in metadata.json).
require('load12.js');
// CHECK:      init seg1.js
// CHECK-NEXT: in export12.js doPrint()
// CHECK-NEXT: init shared12.js
// CHECK-NEXT: shared12.js counter = 1
// CHECK-NEXT: init seg2.js
// CHECK-NEXT: in export12.js doPrint()
// CHECK-NEXT: shared12.js counter = 2

// Load segment 4, then segment 3 (the reverse order from metadata.json).
require('load34.js');
// CHECK-NEXT: init seg4.js
// CHECK-NEXT: in export34.js doPrint()
// CHECK-NEXT: init shared34.js
// CHECK-NEXT: shared34.js counter = 1
// CHECK-NEXT: init seg3.js
// CHECK-NEXT: in export34.js doPrint()
// CHECK-NEXT: shared34.js counter = 2
