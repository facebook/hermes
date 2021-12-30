/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Console assert functionality');
// CHECK-LABEL: Console assert functionality

console.assert(true, 'does nothing');

console.assert(false, 'Whoops %s work', 'didn\'t');
// CHECK-NEXT: Assertion failed: Whoops didn't work

console.assert();
// CHECK-NEXT: Assertion failed
