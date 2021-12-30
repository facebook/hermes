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

print('Incorrect file');
// CHECK-LABEL: Incorrect file

try {
  var test1 = require('./fakeFile.js');
} catch (e) {
  print('caught:', e.name, e.message);
}
//CHECK: caught: Error Failed to open file: {{.*}}
