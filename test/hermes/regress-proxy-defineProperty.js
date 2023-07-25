/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck %s

// Check to ensure that the defineProperty/getOwnPropertyDescriptor
// trap correctly receives either a string or symbol as its
// parameter.
let p = new Proxy({}, {
  defineProperty(target, property, attributes) {
    print(typeof property)
  },
  getOwnPropertyDescriptor(target, property){
    print(typeof property);
  }
})

// getOwnPropertyDescriptor is called as well during setting
// of properties, so expect both print statements to be run.
p.test = 1
// CHECK: string
// CHECK-NEXT: string

p[Symbol('test')] = 1
// CHECK-NEXT: symbol
// CHECK-NEXT: symbol

Object.getOwnPropertyDescriptor(p, 'prop');
// CHECK-NEXT: string

Object.getOwnPropertyDescriptor(p, Symbol('prop'));
// CHECK-NEXT: symbol
