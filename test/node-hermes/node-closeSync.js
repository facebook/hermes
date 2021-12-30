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

print('Node closeSync');
// CHECK-LABEL: Node closeSync

var fs = require('fs');

var fd = fs.openSync('testRead.txt');
fs.closeSync(fd); // Can only just check that there's no exception raised here
