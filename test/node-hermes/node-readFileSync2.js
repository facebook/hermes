/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: diff -B %S/testLongRead.txt <(%node-hermes %s)
// REQUIRES: node-hermes

var fs = require('fs');

var fileContents1 = fs.readFileSync('testLongRead.txt');
print(fileContents1);
