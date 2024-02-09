/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function CHECKED_CAST<T>(x: mixed): T {
  return ((x: any): T);
}

class LinkedList<T> {
  x: T;
  next: LinkedList<T> | null;
  constructor(x: T, next: LinkedList<T> | null) {
    this.x = x;
    this.next = next;
  }
  getNext(): LinkedList<T> {
    if (this.next !== null) {
      return CHECKED_CAST<LinkedList<T>>(this.next);
    }
    throw Error('empty list');
  }
}

print('linkedlist');
// CHECK-LABEL: linkedlist

let n1: LinkedList<number> = new LinkedList<number>(1, null);
let n2: LinkedList<number> = new LinkedList<number>(2, n1);
print(n2.x);
// CHECK-NEXT: 2
print(n1.x);
// CHECK-NEXT: 1
print(n2.getNext().x);
// CHECK-NEXT: 1
