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

print('Console time functionality');
// CHECK-LABEL: Console time functionality

console.time('100-elements');
for (let i = 0; i < 100; i++) {}
console.timeEnd('100-elements');
// CHECK: 100-elements: {{.*}}

function expensiveProcess1(){
    for (let i = 0; i < 10000; i++) {}
    return 42;
}
console.time('process');
const value = expensiveProcess1();
console.timeLog('process', value);
// CHECK-NEXT: process: {{.*}} 42
console.timeEnd('process');
// CHECK-NEXT: process: {{.*}}
