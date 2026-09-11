/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -ferror-limit=0 -typed %s 2>&1) | %FileCheck --match-full-lines %s

type A = {readonly next: A | null, readonly value: number};
type B = {readonly next: B | null, readonly value: string};

function convertObject(value: B): A {
  return value;
}

type FunctionA = (value: FunctionA) => number;
type FunctionB = (value: FunctionB) => string;

function convertFunction(value: FunctionB): FunctionA {
  return value;
}

type CompatibleFunctionA =
    (value: CompatibleFunctionA) => CompatibleFunctionA;
type CompatibleFunctionB =
    (value: CompatibleFunctionB) => CompatibleFunctionB;

function convertCompatibleFunction(
  value: CompatibleFunctionB,
): CompatibleFunctionA {
  return value;
}

type X = {readonly a: P, readonly b: number};
type Y = {readonly a: Q, readonly b: string};
type P = {readonly c: X};
type Q = {readonly c: Y};
type S = {readonly f1: X, readonly f2: P};
type D = {readonly f1: Y | X, readonly f2: Q};

function convertPoisonedCache(value: S): D {
  return value;
}

// CHECK:{{.*}}recursive-flow-error.js:14:3: error: ft: return value incompatible with return type: cannot return object B as object A
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK:{{.*}}recursive-flow-error.js:21:3: error: ft: return value incompatible with return type: cannot return function FunctionB as function FunctionA
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK:{{.*}}recursive-flow-error.js:43:3: error: ft: return value incompatible with return type: cannot return object S as object D
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
