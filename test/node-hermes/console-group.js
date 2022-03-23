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

print('Console group functionality');
// CHECK-LABEL: Console group functionality

console.log("This is the outer level");
// CHECK: This is the outer level
console.group();
console.log("Level 2");
// CHECK-NEXT: Level 2
console.group();
console.log("Level 3");
// CHECK-NEXT: Level 3
console.log("More of level 3");
// CHECK-NEXT: More of level 3
console.groupEnd();
console.log("Back to level 2");
// CHECK-NEXT: Back to level 2
console.groupEnd();
console.log("Back to the outer level");
// CHECK-NEXT: Back to the outer level
