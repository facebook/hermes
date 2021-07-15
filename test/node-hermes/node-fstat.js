/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Node fstatSync');
// CHECK-LABEL: Node fstatSync

var fs = require('fs');

var fd = fs.openSync('testRead.txt', 'r', 438);

var binding = internalBinding('fs');
var ctx = {};
var stats = binding.fstat(fd, false, undefined, ctx);
print(stats.size);
// CHECK: 13
