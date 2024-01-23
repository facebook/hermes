/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-class %s | %FileCheck --match-full-lines %s
// REQUIRES: es6_class

class Parent {}

class Child extends Parent {}

const parentInstance = new Parent();

print(parentInstance instanceof Parent);
//CHECK: true

print(parentInstance instanceof Child);
//CHECK: false

const childInstance = new Child();

print(childInstance instanceof Parent);
//CHECK: true

print(childInstance instanceof Child);
//CHECK: true
