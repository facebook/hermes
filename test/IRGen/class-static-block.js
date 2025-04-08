/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

class A {
  static #f1 = 10;
  static {
    var x = this.#f1;
  }
  static {
    var x = this.#f1;
  }
  static {
    let y = this.#f1;
    print(y);
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [A: any, A#1: any, #f1: privateName, ?A.prototype: object, ?A: object]

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.A]: any
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.A#1]: any
// CHECK-NEXT:  %5 = CreatePrivateNameInst (:privateName) "#f1": string
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: privateName, [%VS0.#f1]: privateName
// CHECK-NEXT:  %7 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %8 = CreateClassInst (:object) %0: environment, %VS0: any, %A(): functionCode, empty: any, %7: object
// CHECK-NEXT:  %9 = LoadStackInst (:object) %7: object
// CHECK-NEXT:        StoreFrameInst %0: environment, %8: object, [%VS0.A#1]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, %8: object, [%VS0.?A]: object
// CHECK-NEXT:        StoreFrameInst %0: environment, %9: object, [%VS0.?A.prototype]: object
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %<static_elements_initializer:A>(): functionCode
// CHECK-NEXT:  %14 = CallInst (:any) %13: object, %<static_elements_initializer:A>(): functionCode, true: boolean, %0: environment, undefined: undefined, %8: object
// CHECK-NEXT:        StoreFrameInst %0: environment, %8: object, [%VS0.A]: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:base constructor A(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function <static_elements_initializer:A>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS0.?A]: object
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %1: environment, [%VS0.#f1]: privateName
// CHECK-NEXT:       AddOwnPrivateFieldInst 10: number, %3: object, %4: privateName, false: boolean
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %2: environment, %VS2: any, %<A:static_block_0>(): functionCode
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, %<A:static_block_0>(): functionCode, true: boolean, %2: environment, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %2: environment, %VS2: any, %<A:static_block_1>(): functionCode
// CHECK-NEXT:  %9 = CallInst (:any) %8: object, %<A:static_block_1>(): functionCode, true: boolean, %2: environment, undefined: undefined, %0: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %2: environment, %VS2: any, %<A:static_block_2>(): functionCode
// CHECK-NEXT:  %11 = CallInst (:any) %10: object, %<A:static_block_2>(): functionCode, true: boolean, %2: environment, undefined: undefined, %0: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any]

// CHECK:function <A:static_block_0>(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:       StoreFrameInst %2: environment, undefined: undefined, [%VS3.x]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %VS0: any, %VS2: any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:privateName) %4: environment, [%VS0.#f1]: privateName
// CHECK-NEXT:  %6 = LoadOwnPrivateFieldInst (:any) %0: any, %5: privateName
// CHECK-NEXT:       StoreFrameInst %2: environment, %6: any, [%VS3.x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any]

// CHECK:function <A:static_block_1>(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:       StoreFrameInst %2: environment, undefined: undefined, [%VS4.x]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %VS0: any, %VS2: any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:privateName) %4: environment, [%VS0.#f1]: privateName
// CHECK-NEXT:  %6 = LoadOwnPrivateFieldInst (:any) %0: any, %5: privateName
// CHECK-NEXT:       StoreFrameInst %2: environment, %6: any, [%VS4.x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [y: any]

// CHECK:function <A:static_block_2>(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:       StoreFrameInst %2: environment, undefined: undefined, [%VS5.y]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %VS0: any, %VS2: any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:privateName) %4: environment, [%VS0.#f1]: privateName
// CHECK-NEXT:  %6 = LoadOwnPrivateFieldInst (:any) %0: any, %5: privateName
// CHECK-NEXT:       StoreFrameInst %2: environment, %6: any, [%VS5.y]: any
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS5.y]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %9: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
