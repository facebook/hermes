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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "load_only_capture": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "load_dedup": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %load_only_capture(): number
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: object, globalObject: object, "load_only_capture": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %load_dedup(): object
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "load_dedup": string
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function load_only_capture(leak: any, foreach: any, n: any): number
// CHECK-NEXT:frame = [i: number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %leak: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %n: any
// CHECK-NEXT:  %2 = StoreFrameInst 0: number, [i]: number
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) 0: number, %1: any
// CHECK-NEXT:  %4 = CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst (:number) 0: number, %BB0, %8: number, %BB1
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %""(): number
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: object, %0: any, "k": string
// CHECK-NEXT:  %8 = FAddInst (:number) %5: number, 1: number
// CHECK-NEXT:  %9 = StoreFrameInst %8: number, [i]: number
// CHECK-NEXT:  %10 = BinaryLessThanInst (:boolean) %8: number, %1: any
// CHECK-NEXT:  %11 = CondBranchInst %10: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = PhiInst (:number) 0: number, %BB0, %8: number, %BB1
// CHECK-NEXT:  %13 = ReturnInst %12: number
// CHECK-NEXT:function_end

// CHECK:function load_dedup(foo: any): object
// CHECK-NEXT:frame = [foo: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %foo: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [foo]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %bar(): any
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:number) [i@load_only_capture]: number
// CHECK-NEXT:  %1 = ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [foo@load_dedup]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [x@load_dedup]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:  %3 = ReturnInst %1: any
// CHECK-NEXT:function_end
