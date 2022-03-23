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

print('Path test 1');
// CHECK-LABEL: Path test 1

var test = require('../example-exports.js');
print(test.x);
// CHECK-NEXT: 5
print(test.y);
// CHECK-NEXT: 3
