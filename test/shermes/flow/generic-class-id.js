/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class ID<T> {
  val: T;

  constructor(val: T) {
    this.val = val;
  }
}

print('generic class');
// CHECK-LABEL: generic class

const i1: ID<number> = new ID<number>(1);
const n: number = i1.val;
print(n);
// CHECK-NEXT: 1

const i2: ID<string> = new ID<string>('abc');
const s: string = i2.val;
print(s);
// CHECK-NEXT: abc
