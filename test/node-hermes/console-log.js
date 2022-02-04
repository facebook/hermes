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

print('Console log functionality');
// CHECK-LABEL: Console log functionality

console.log('hello %s %d', 'world', 123);
// CHECK: hello world 123

console.log('hello %s %d', 'world again ', 123);
// CHECK-NEXT: hello world again 123

console.log("hello %s %d", 'world', 123);
// CHECK-NEXT: hello world 123

var {Console} = require('console');
var console1 = Console({stdout: process.stdout, stderr: process.stderr});
console1.log('hello %s %d', 'world', 123);
// CHECK-NEXT: hello world 123

var {Console} = console;
var console2 = Console({stdout: process.stdout, stderr: process.stderr});
console2.log('hello %s %d', 'world', 123);
// CHECK-NEXT: hello world 123
