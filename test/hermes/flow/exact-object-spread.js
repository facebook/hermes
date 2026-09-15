/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

print('spread');
// CHECK-LABEL: spread

let src: {x: number, y: string} = {x: 1, y: "hi"};

// Pure spread copies all fields.
let pure: {x: number, y: string} = {...src};
print(pure.x, pure.y);
// CHECK-NEXT: 1 hi

// Explicit property after spread overrides spread value.
let over: {x: number, y: string} = {...src, x: 99};
print(over.x, over.y);
// CHECK-NEXT: 99 hi

// Spread after explicit property wins.
let pre: {x: number, y: string} = {x: 99, ...src};
print(pre.x, pre.y);
// CHECK-NEXT: 1 hi

// Two spreads merged.
let a: {x: number} = {x: 7};
let b: {y: string} = {y: "merged"};
let merged: {x: number, y: string} = {...a, ...b};
print(merged.x, merged.y);
// CHECK-NEXT: 7 merged

// Mutating the spread result does not mutate the source.
pure.x = 42;
print(pure.x, src.x);
// CHECK-NEXT: 42 1
