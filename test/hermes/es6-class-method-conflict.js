/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

function multiply(left, right) {
  return left * right;
}

class Number {

  constructor(value) {
    this.value = value;
  }

  multiply(value) {
    this.value = multiply(this.value, value);
    return this;
  }
}

const number = new Number(4);

number.multiply(2);
print(number.value);
//CHECK: 8
