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

print('Console error functionality');
// CHECK-LABEL: Console error functionality

var name = 'Will Robinson';
console.warn(`Danger ${name}! Danger!`);
// CHECK: Danger Will Robinson! Danger!

var code = 5;
console.error('error #%d', code);
// CHECK-NEXT: error #5
