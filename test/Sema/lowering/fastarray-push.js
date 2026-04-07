/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-transformed-ast %s | %FileCheck --match-full-lines %s

// Test lowering of fastarray push

var x: number[];
x.push(10, 20);

// Use "raw": "10" as a unique anchor that only appears in the test's code.
// CHECK:              "name": "?fastArrayPush"
// CHECK:              "name": "x"
// CHECK:              "raw": "10"
// CHECK:              "raw": "20"
