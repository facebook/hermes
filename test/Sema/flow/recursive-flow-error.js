/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -fno-std-globals -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

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

type X = {readonly a: P, readonly b: number};
type Y = {readonly a: Q, readonly b: string};
type P = {readonly c: X};
type Q = {readonly c: Y};
type S = {readonly f1: X, readonly f2: P};
type D = {readonly f1: Y | X, readonly f2: Q};

function convertSharedSubterm(value: S): D {
  return value;
}

// The 'any' arm of AnyUnion needs a checked cast, which a readonly field
// cannot emit. The cycle assumption must not hide that from the field check.
type AnyHolder = {readonly f: AnyUnion};
type AnyUnion = AnyHolder | any;
type AnyTarget = {readonly f: AnyTarget};

function convertAnyArm(value: AnyUnion): AnyTarget {
  return value;
}

// ArityA -> ArityB needs a checked cast for the extra optional parameter, so
// it cannot satisfy the no-cast return type check that recurses back into it.
type ArityA = (p?: number) => (ArityA | number);
type ArityB = () => (ArityB | number);

function convertArity(value: ArityA): ArityB {
  return value;
}

// The override must be rejected: A cannot flow into B (number vs string), so
// the recursive parameter check has to fail instead of assuming the cycle
// flows.
class OverrideBase {
  method(value: A): A {
    return value;
  }
}

class OverrideDerived extends OverrideBase {
  method(value: B): A {
    throw 0;
  }
}

// Checking X == Y assumes P == Q while in progress; that assumption fails on
// `b`, so P == Q must not be remembered as true when the unions are compared.
type UnionObj1 = X | P | Y;
type UnionObj2 = Y | X | Q;

function convertUnionObj(value: {f: UnionObj1}): {f: UnionObj2} {
  return value;
}

// Same, with the cycle closed through function return types.
type RetF = () => RetP;
type RetG = () => RetQ;
type RetP = {a: RetR, b: number};
type RetQ = {a: RetS, b: string};
type RetR = {c: RetF};
type RetS = {c: RetG};
type UnionRet1 = RetF | RetG | RetR;
type UnionRet2 = RetG | RetF | RetS;

function convertUnionRet(value: {u: UnionRet1}): {u: UnionRet2} {
  return value;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}recursive-flow-error.js:14:3: error: ft: return value incompatible with return type: cannot return object B as object A
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:21:3: error: ft: return value incompatible with return type: cannot return function FunctionB as function FunctionA
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:32:3: error: ft: return value incompatible with return type: cannot return object S as object D
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:42:3: error: ft: return value incompatible with return type: cannot return union AnyUnion as object AnyTarget
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:51:3: error: ft: return value incompatible with return type: cannot return function ArityA as function ArityB
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:64:3: error: ft: incompatible method type for override
// CHECK-NEXT:  method(value: B): A {
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}recursive-flow-error.js:75:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}recursive-flow-error.js:89:3: error: ft: return value incompatible with return type: cannot return object as object
// CHECK-NEXT:  return value;
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:Emitted 8 errors. exiting.
