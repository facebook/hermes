/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function t(x) {
  if (typeof x === 'undefined') f1();
  if (typeof x === 'object')    f2();
  if (typeof x !== 'object')    f3();
  if (typeof x == 'symbol')     f4();
  if (typeof x != 'bigint')     f5();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "t": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %t(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "t": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function t(x: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = TypeOfIsInst (:boolean) %0: any, typeOfIs(Undefined)
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "f1": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = TypeOfIsInst (:boolean) %0: any, typeOfIs(Object,Null)
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "f2": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = TypeOfIsInst (:boolean) %0: any, typeOfIs(Undefined,String,Symbol,Boolean,Number,Bigint,Function)
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB5, %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst (:any) globalObject: object, "f3": string
// CHECK-NEXT:  %14 = CallInst (:any) %13: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = TypeOfIsInst (:boolean) %0: any, typeOfIs(Symbol)
// CHECK-NEXT:        CondBranchInst %16: boolean, %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "f4": string
// CHECK-NEXT:  %19 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %21 = TypeOfIsInst (:boolean) %0: any, typeOfIs(Undefined,Object,String,Symbol,Boolean,Number,Function,Null)
// CHECK-NEXT:        CondBranchInst %21: boolean, %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "f5": string
// CHECK-NEXT:  %24 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
