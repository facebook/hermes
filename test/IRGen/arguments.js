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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "cheap": string
// CHECK-NEXT:       DeclareGlobalVarInst "expensive": string
// CHECK-NEXT:       DeclareGlobalVarInst "cond": string
// CHECK-NEXT:       DeclareGlobalVarInst "shadow": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %cheap(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "cheap": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %expensive(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "expensive": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %cond(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "cond": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %shadow(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "shadow": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function cheap(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %cheap(): any, %1: environment
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %3: any, %4: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function expensive(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %expensive(): any, %1: environment
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "cheap": string
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: object
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %3: any, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %0: object, 0: number
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function cond(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %cond(): any, %1: environment
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: object, "length": string
// CHECK-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: object, 1: number
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function shadow(arguments: any): any
// CHECK-NEXT:frame = [arguments: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %shadow(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %arguments: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [arguments]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [arguments]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "length": string
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end
