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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "store_dedup": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %store_dedup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "store_dedup": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_dedup(foo: any): any
// CHECK-NEXT:frame = [foo: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %store_dedup(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [foo]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %store_dedup(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [foo@store_dedup]: any
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: any, [x@store_dedup]: any
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, %4: any, [x@store_dedup]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
