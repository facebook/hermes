/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

print('string-property');
// CHECK-LABEL: string-property

let s: string = "hello";
print(s.length);
// CHECK-NEXT: 5

print(s[0]);
// CHECK-NEXT: h

print(s[4]);
// CHECK-NEXT: o

let len: number = s.length;
print(len);
// CHECK-NEXT: 5

let ch: string = s[1];
print(ch);
// CHECK-NEXT: e
