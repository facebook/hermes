/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Basic Requires');
// CHECK-LABEL: Basic Requires

var test1 = require('./example-exports.js');
print(typeof require);
// CHECK-NEXT: function
print(test1.x);
// CHECK-NEXT: 5
print(test1.y);
// CHECK-NEXT: 3

var test2 = require('./example-exports.js');
print(test2.x);
// CHECK-NEXT: 5
print(test2.y);
// CHECK-NEXT: 3
