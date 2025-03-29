/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines

function simpleFields() {
  class A {
    static #privateF1 = 10;
    f2 = 20;
    f3 = 30;
    #privateF4 = 40;

    static checkF1(o) {
      return #privateF1 in o;
    }
    checkF4() {
      return #privateF4 in this;
    }
  }
  return A;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simpleFields": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simpleFields(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "simpleFields": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [A: any, A#1: any, #privateF1: privateName, #privateF4: privateName, ?A.prototype: object, ?A: object, <instElemInitFunc:A>: object]

// CHECK:function simpleFields(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.A]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.A#1]: any
// CHECK-NEXT:  %4 = CreatePrivateNameInst (:privateName) "#privateF1": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: privateName, [%VS1.#privateF1]: privateName
// CHECK-NEXT:  %6 = CreatePrivateNameInst (:privateName) "#privateF4": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: privateName, [%VS1.#privateF4]: privateName
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<instance_members_initializer:A>(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.<instElemInitFunc:A>]: object
// CHECK-NEXT:  %10 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %11 = CreateClassInst (:object) %1: environment, %VS1: any, %A(): functionCode, empty: any, %10: object
// CHECK-NEXT:  %12 = LoadStackInst (:object) %10: object
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %1: environment, %VS1: any, %checkF1(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %13: object, %11: object, "checkF1": string, false: boolean
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %1: environment, %VS1: any, %checkF4(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %15: object, %12: object, "checkF4": string, false: boolean
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.A#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.?A]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.?A.prototype]: object
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<static_elements_initializer:A>(): functionCode
// CHECK-NEXT:  %21 = CallInst (:any) %20: object, %<static_elements_initializer:A>(): functionCode, true: boolean, %1: environment, undefined: undefined, %11: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.A]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [%VS1.A]: any
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function <instance_members_initializer:A>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:       DefineOwnPropertyInst 20: number, %0: any, "f2": string, true: boolean
// CHECK-NEXT:       DefineOwnPropertyInst 30: number, %0: any, "f3": string, true: boolean
// CHECK-NEXT:  %5 = LoadFrameInst (:privateName) %1: environment, [%VS1.#privateF4]: privateName
// CHECK-NEXT:  %6 = BinaryPrivateInInst (:any) %5: privateName, %0: any
// CHECK-NEXT:       CondBranchInst %6: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       AddOwnPrivateFieldInst 40: number, %0: any, %5: privateName, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ThrowTypeErrorInst "Cannot initialize private field twice.": string
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:base constructor A(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS1.<instElemInitFunc:A>]: object
// CHECK-NEXT:  %4 = CallInst (:undefined) %3: object, empty: any, true: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [o: any]

// CHECK:method checkF1(o: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS1.#privateF1]: privateName
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS4.o]: any
// CHECK-NEXT:  %6 = BinaryPrivateInInst (:boolean) %4: privateName, %5: any
// CHECK-NEXT:       ReturnInst %6: boolean
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:method checkF4(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:privateName) %1: environment, [%VS1.#privateF4]: privateName
// CHECK-NEXT:  %4 = BinaryPrivateInInst (:boolean) %3: privateName, %0: any
// CHECK-NEXT:       ReturnInst %4: boolean
// CHECK-NEXT:function_end

// CHECK:scope %VS6 []

// CHECK:function <static_elements_initializer:A>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.?A]: object
// CHECK-NEXT:  %3 = LoadFrameInst (:privateName) %0: environment, [%VS1.#privateF1]: privateName
// CHECK-NEXT:       AddOwnPrivateFieldInst 10: number, %2: object, %3: privateName, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
