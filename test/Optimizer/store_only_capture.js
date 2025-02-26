/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function store_dedup(foo){
    var x;
    function bar(){
        x = foo();
        x = foo();
    }
    foo(bar);
    return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "store_dedup": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %store_dedup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "store_dedup": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [foo: any, x: any]

// CHECK:function store_dedup(foo: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS0.foo]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS0: any, %bar(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %0: environment, [%VS0.x]: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [%VS0.foo]: any
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: any, [%VS0.x]: any
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, %4: any, [%VS0.x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
