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

print('Node openSync');
// CHECK-LABEL: Node openSync

var fs = require('fs');

var fd = fs.openSync('testRead.txt');
print(typeof fd == 'number');
//CHECK: true
