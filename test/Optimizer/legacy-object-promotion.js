/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// We use this as a sink because it's a global function, whose usages
// cannot be analyzed further.
function sink(x) { return x; }

function simple() {
  var obj = {
    a: 1
  };
  return obj.a;
}

function loadsAndStores() {
  var obj = {
    a: 1,
    b: 2,
    c: 3,
    d: sink(4),
  };
  obj.a += 1;
  obj.b += 1;
  obj.c += 1;
  obj.d += 1;
  return obj.a + obj.b + obj.c + obj.d;
}

function numericProperties() {
  var obj = {
    0: 'a',
    1: 'b',
    2: 'c',
    3.14: 'pi',
  };
  obj[0] += 'd';
  obj[1] += 'e';
  obj[2] += 'f';
  obj[3.14] += 'g';
  return obj[0] + obj[1] + obj[2] + obj[3.14];
}

function escapingRet() {
  var obj = {
    a: 1
  };
  return obj;
}

function escapingFuncCall() {
  var obj = {
    a: 1
  };
  sink(obj);
  return obj.a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple": string
// CHECK-NEXT:       DeclareGlobalVarInst "loadsAndStores": string
// CHECK-NEXT:       DeclareGlobalVarInst "numericProperties": string
// CHECK-NEXT:       DeclareGlobalVarInst "escapingRet": string
// CHECK-NEXT:       DeclareGlobalVarInst "escapingFuncCall": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "sink": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %simple(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "simple": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) empty: any, empty: any, %loadsAndStores(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "loadsAndStores": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) empty: any, empty: any, %numericProperties(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "numericProperties": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) empty: any, empty: any, %escapingRet(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "escapingRet": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) empty: any, empty: any, %escapingFuncCall(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "escapingFuncCall": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function simple(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function loadsAndStores(): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number) %1: any, 1: number
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) 9: number, %2: string|number
// CHECK-NEXT:       ReturnInst %3: string|number
// CHECK-NEXT:function_end

// CHECK:function numericProperties(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst "adbecfpig": string
// CHECK-NEXT:function_end

// CHECK:function escapingRet(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, "a": string, 1: number
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function escapingFuncCall(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, "a": string, 1: number
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %0: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, "a": string
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end
