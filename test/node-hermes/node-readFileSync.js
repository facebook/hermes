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

print('Node readFileSync');
// CHECK-LABEL: Node readFileSync

var fs = require('fs');

var fileContents1 = fs.readFileSync('testRead.txt');
print(fileContents1);
//CHECK: Hello world!

try {
  var fileContents2 = fs.readFileSync('fileDoesNotExist.txt');
} catch (e) {
  print('caught:', e.name, e.message);
}
//CHECK: caught: Error OpenSync failed on file 'fileDoesNotExist.txt' {{.*}}
