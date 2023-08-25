/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir -non-strict %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir -non-strict %s -O

// Code that doesn't require creation of the expensive object.
function cheap() {
    return arguments.length + arguments[0];
}

// Code that requires creation of the expensive object.
function expensive() {
    return arguments.length + cheap(arguments) + arguments[0];
}

// Something with more than one BB
function cond() {
    if (arguments.length)
        return arguments[1];
}

function shadow(arguments) {
    return arguments.length;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "cheap": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "expensive": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "cond": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "shadow": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %cheap(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "cheap": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %expensive(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: object, globalObject: object, "expensive": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %cond(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "cond": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %shadow(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "shadow": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function cheap(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %3 = BinaryAddInst (:any) %1: any, %2: any
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function expensive(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "cheap": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: object
// CHECK-NEXT:  %4 = BinaryAddInst (:any) %1: any, %3: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function cond(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:  %2 = CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, 1: number
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function shadow(arguments: any): any
// CHECK-NEXT:frame = [arguments: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %arguments: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [arguments]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [arguments]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "length": string
// CHECK-NEXT:  %4 = ReturnInst %3: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = UnreachableInst
// CHECK-NEXT:function_end
