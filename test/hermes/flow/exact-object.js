/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

print('objects');
// CHECK-LABEL: objects

let t: {x: number, y: string} = {x: 3, y: "hello"};
print(t.x, t.y);
// CHECK-NEXT: 3 hello
let tx: number = t.x;
t.x = 5;
print(t.x);
// CHECK-NEXT: 5
let t_any: any = t;
print(t_any.x);
// CHECK-NEXT: 5
let t2: {x: number, y: string} = {x: 3, x: 5, y: t.y, y: "hello"};
print(t.x, t.y);
// CHECK-NEXT: 5 hello
