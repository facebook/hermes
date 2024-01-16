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
// CHECK-NEXT:       DeclareGlobalVarInst "store_dedup": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %store_dedup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "store_dedup": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function store_dedup(foo: any): any
// CHECK-NEXT:frame = [foo: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:       StoreFrameInst %0: any, [foo]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %bar(): functionCode
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [foo@store_dedup]: any
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: any, [x@store_dedup]: any
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %3: any, [x@store_dedup]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
