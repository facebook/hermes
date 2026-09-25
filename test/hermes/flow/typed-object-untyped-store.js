/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %hermes -typed -O0 %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Untyped stores to a typed object must throw, even when inlining lets
// ObjectStackPromotion see the allocation and the store in the same function.

class TestClass {
  myName: string;
  constructor(name: string) {
    this.myName = name;
  }
}

function writeToMyName(target, value) {
  target.myName = value;
}

const testObject = new TestClass('Dracula');
try {
  writeToMyName(testObject, 'Wolf Man');
  print('no error');
} catch (e) {
  print(e.name, e.message);
}
// CHECK: TypeError Cannot assign to read-only property 'myName'
print(testObject.myName);
// CHECK-NEXT: Dracula
