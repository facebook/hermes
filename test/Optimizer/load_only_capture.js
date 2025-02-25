/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function load_only_capture(leak, foreach, n) {
    for(var i = 0; i < n; i++){
      leak.k = () => i;
    }
    return i;
}

function load_dedup(foo){
    var x = foo();
    function bar(){
        foo(x);
        return x;
    }
    return bar;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "load_only_capture": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_dedup": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %load_only_capture(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "load_only_capture": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %load_dedup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "load_dedup": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [i: number]

// CHECK:function load_only_capture(leak: any, foreach: any, n: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %leak: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS1.i]: number
// CHECK-NEXT:  %4 = BinaryLessThanInst (:boolean) 0: number, %2: any
// CHECK-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst (:number) 0: number, %BB0, %9: number, %BB1
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS1: any, %""(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, %1: any, "k": string
// CHECK-NEXT:  %9 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %9: number, [%VS1.i]: number
// CHECK-NEXT:  %11 = BinaryLessThanInst (:boolean) %9: number, %2: any
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = PhiInst (:number) 0: number, %BB0, %9: number, %BB1
// CHECK-NEXT:        ReturnInst %13: number
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [foo: any, x: any]

// CHECK:function load_dedup(foo: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS2: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS2.foo]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS2.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS2: any, %bar(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [%VS2.x]: any
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:any) %0: environment, [%VS2.foo]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS2.x]: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end
