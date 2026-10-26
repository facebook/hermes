/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s -dump-ir | %FileCheckOrRegen --match-full-lines %s

// Regression test: async function declared in a finally block.
// The finally body is emitted once per control-flow exit path (return, normal
// completion, rethrow), but the async outer function is compiled only once and
// loads a single hidden "_innerClosure" variable. Previously only the first
// copy emitted the store into that variable, so the other paths passed an
// uninitialized value to spawnAsync. All three copies must store into it.

function main(cond) {
    try {
        if (cond) return 1;
    } finally {
        async function g() { return 2; }
        g();
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [cond: any, g: any, g#1: any, ?anon_0_g_innerClosure: object]

// CHECK:function main(cond: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.cond]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.g]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CatchInst (:any)
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %VS1: any, %?anon_1_g(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS1.?anon_0_g_innerClosure]: object
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %1: environment, %VS1: any, %g(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.g]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.g#1]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.g#1]: any
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ThrowInst %6: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS1.cond]: any
// CHECK-NEXT:        CondBranchInst %16: any, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst %BB1, %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst %BB1, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %1: environment, %VS1: any, %?anon_1_g(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: object, [%VS1.?anon_0_g_innerClosure]: object
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %1: environment, %VS1: any, %g(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: object, [%VS1.g]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: object, [%VS1.g#1]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [%VS1.g#1]: any
// CHECK-NEXT:  %27 = CallInst (:any) %26: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst 1: number
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = CreateFunctionInst (:object) %1: environment, %VS1: any, %?anon_1_g(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: object, [%VS1.?anon_0_g_innerClosure]: object
// CHECK-NEXT:  %31 = CreateFunctionInst (:object) %1: environment, %VS1: any, %g(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: object, [%VS1.g]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: object, [%VS1.g#1]: any
// CHECK-NEXT:  %34 = LoadFrameInst (:any) %1: environment, [%VS1.g#1]: any
// CHECK-NEXT:  %35 = CallInst (:any) %34: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function ?anon_1_g(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateGeneratorInst (:object) %1: environment, %VS1: any, %"?anon_1_g 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function g(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS2: any, %3: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:object) %3: environment, [%VS1.?anon_0_g_innerClosure]: object
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [HermesBuiltin.spawnAsync]: number
// CHECK-NEXT:  %7 = CallInst (:any) %6: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object, %2: object, %0: object
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:generator inner "?anon_1_g 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:boolean) $?anon_0_isReturn_prologue: any
// CHECK-NEXT:  %1 = ResumeGeneratorInst (:any) %0: boolean
// CHECK-NEXT:  %2 = LoadStackInst (:boolean) %0: boolean
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %5 = CreateScopeInst (:environment) %VS3: any, %4: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end
