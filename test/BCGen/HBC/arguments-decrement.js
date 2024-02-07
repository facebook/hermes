/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xdump-between-passes %s 2>&1 | %FileCheckOrRegen --match-full-lines %s --check-prefix CHKIR
// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s --check-prefix CHKRA

// This test exercises an issue found in LowerArgumentsArray in which PHI nodes
// were not being properly updated.
function decrementArguments() {
    for (var i = 0; i < 2; i++) {
        var var1 = () => var3 = 0;
        var var3 = arguments;
    }
    return var3 - 1;
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:*** INITIAL STATE

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any, var1: any, var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHKIR-NEXT:       StoreFrameInst %2: object, [?anon_0_this]: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst %4: undefined|object, [?anon_1_new.target]: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var1]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreFrameInst 0: number, [i]: any
// CHKIR-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 2: number
// CHKIR-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %13: object, [var1]: any
// CHKIR-NEXT:        StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:        BranchInst %BB3
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %17 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %18 = BinarySubtractInst (:any) %17: any, 1: number
// CHKIR-NEXT:        ReturnInst %18: any
// CHKIR-NEXT:%BB4:
// CHKIR-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %21 = BinaryLessThanInst (:boolean) %20: any, 2: number
// CHKIR-NEXT:        CondBranchInst %21: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB3:
// CHKIR-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %24 = AsNumericInst (:number|bigint) %23: any
// CHKIR-NEXT:  %25 = UnaryIncInst (:number|bigint) %24: number|bigint
// CHKIR-NEXT:        StoreFrameInst %25: number|bigint, [i]: any
// CHKIR-NEXT:        BranchInst %BB4
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER InstSimplify

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any, var1: any, var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHKIR-NEXT:       StoreFrameInst %2: object, [?anon_0_this]: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst %4: undefined|object, [?anon_1_new.target]: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var1]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreFrameInst 0: number, [i]: any
// CHKIR-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 2: number
// CHKIR-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %13: object, [var1]: any
// CHKIR-NEXT:        StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:        BranchInst %BB3
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %17 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %18 = BinarySubtractInst (:any) %17: any, 1: number
// CHKIR-NEXT:        ReturnInst %18: any
// CHKIR-NEXT:%BB4:
// CHKIR-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %21 = BinaryLessThanInst (:boolean) %20: any, 2: number
// CHKIR-NEXT:        CondBranchInst %21: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB3:
// CHKIR-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: any
// CHKIR-NEXT:        StoreFrameInst %24: number|bigint, [i]: any
// CHKIR-NEXT:        BranchInst %BB4
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER ResolveStaticRequire

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any, var1: any, var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHKIR-NEXT:       StoreFrameInst %2: object, [?anon_0_this]: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst %4: undefined|object, [?anon_1_new.target]: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var1]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreFrameInst 0: number, [i]: any
// CHKIR-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 2: number
// CHKIR-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %13: object, [var1]: any
// CHKIR-NEXT:        StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:        BranchInst %BB3
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %17 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %18 = BinarySubtractInst (:any) %17: any, 1: number
// CHKIR-NEXT:        ReturnInst %18: any
// CHKIR-NEXT:%BB4:
// CHKIR-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %21 = BinaryLessThanInst (:boolean) %20: any, 2: number
// CHKIR-NEXT:        CondBranchInst %21: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB3:
// CHKIR-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: any
// CHKIR-NEXT:        StoreFrameInst %24: number|bigint, [i]: any
// CHKIR-NEXT:        BranchInst %BB4
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DCE

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any, var1: any, var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHKIR-NEXT:       StoreFrameInst %2: object, [?anon_0_this]: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst %4: undefined|object, [?anon_1_new.target]: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var1]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreFrameInst 0: number, [i]: any
// CHKIR-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 2: number
// CHKIR-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %13: object, [var1]: any
// CHKIR-NEXT:        StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:        BranchInst %BB3
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %17 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %18 = BinarySubtractInst (:any) %17: any, 1: number
// CHKIR-NEXT:        ReturnInst %18: any
// CHKIR-NEXT:%BB4:
// CHKIR-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %21 = BinaryLessThanInst (:boolean) %20: any, 2: number
// CHKIR-NEXT:        CondBranchInst %21: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB3:
// CHKIR-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: any
// CHKIR-NEXT:        StoreFrameInst %24: number|bigint, [i]: any
// CHKIR-NEXT:        BranchInst %BB4
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimplifyCFG

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any, var1: any, var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHKIR-NEXT:       StoreFrameInst %2: object, [?anon_0_this]: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst %4: undefined|object, [?anon_1_new.target]: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var1]: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreFrameInst 0: number, [i]: any
// CHKIR-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 2: number
// CHKIR-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %13 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %13: object, [var1]: any
// CHKIR-NEXT:        StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:  %16 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: any
// CHKIR-NEXT:        StoreFrameInst %17: number|bigint, [i]: any
// CHKIR-NEXT:  %19 = LoadFrameInst (:any) [i]: any
// CHKIR-NEXT:  %20 = BinaryLessThanInst (:boolean) %19: any, 2: number
// CHKIR-NEXT:        CondBranchInst %20: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %22 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %23 = BinarySubtractInst (:any) %22: any, 1: number
// CHKIR-NEXT:        ReturnInst %23: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimpleStackPromotion

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreStackInst 0: number, %0: any
// CHKIR-NEXT:  %8 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %9 = BinaryLessThanInst (:boolean) %8: any, 2: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %11 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %13 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %14 = UnaryIncInst (:number|bigint) %13: any
// CHKIR-NEXT:        StoreStackInst %14: number|bigint, %0: any
// CHKIR-NEXT:  %16 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %17 = BinaryLessThanInst (:boolean) %16: any, 2: number
// CHKIR-NEXT:        CondBranchInst %17: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %19 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %20 = BinarySubtractInst (:any) %19: any, 1: number
// CHKIR-NEXT:        ReturnInst %20: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FrameLoadStoreOpts

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHKIR-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKIR-NEXT:       ReturnInst %5: any
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:       StoreStackInst 0: number, %0: any
// CHKIR-NEXT:  %8 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %9 = BinaryLessThanInst (:boolean) %8: any, 2: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %11 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %13 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %14 = UnaryIncInst (:number|bigint) %13: any
// CHKIR-NEXT:        StoreStackInst %14: number|bigint, %0: any
// CHKIR-NEXT:  %16 = LoadStackInst (:any) %0: any
// CHKIR-NEXT:  %17 = BinaryLessThanInst (:boolean) %16: any, 2: number
// CHKIR-NEXT:        CondBranchInst %17: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %19 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %20 = BinarySubtractInst (:any) %19: any, 1: number
// CHKIR-NEXT:        ReturnInst %20: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Mem2Reg

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %6 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %9 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %11 = UnaryIncInst (:number|bigint) %8: any
// CHKIR-NEXT:  %12 = BinaryLessThanInst (:boolean) %11: number|bigint, 2: number
// CHKIR-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %14 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %15 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %16 = BinarySubtractInst (:any) %15: any, 1: number
// CHKIR-NEXT:        ReturnInst %16: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimpleStackPromotion

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %6 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %9 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %11 = UnaryIncInst (:number|bigint) %8: any
// CHKIR-NEXT:  %12 = BinaryLessThanInst (:boolean) %11: number|bigint, 2: number
// CHKIR-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %14 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %15 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %16 = BinarySubtractInst (:any) %15: any, 1: number
// CHKIR-NEXT:        ReturnInst %16: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FunctionAnalysis

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %6 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %9 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %11 = UnaryIncInst (:number|bigint) %8: any
// CHKIR-NEXT:  %12 = BinaryLessThanInst (:boolean) %11: number|bigint, 2: number
// CHKIR-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %14 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %15 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %16 = BinarySubtractInst (:any) %15: any, 1: number
// CHKIR-NEXT:        ReturnInst %16: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any [allCallsitesKnownInStrictMode]
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Inlining

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:any) $i: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:  %2 = LoadParamInst (:any) %<this>: any
// CHKIR-NEXT:  %3 = CoerceThisNSInst (:object) %2: any
// CHKIR-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %6 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %9 = CreateFunctionInst (:object) %var1(): functionCode
// CHKIR-NEXT:        StoreFrameInst %1: object, [var3]: any
// CHKIR-NEXT:  %11 = UnaryIncInst (:number|bigint) %8: any
// CHKIR-NEXT:  %12 = BinaryLessThanInst (:boolean) %11: number|bigint, 2: number
// CHKIR-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %14 = PhiInst (:any) 0: number, %BB0, %11: number|bigint, %BB1
// CHKIR-NEXT:  %15 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %16 = BinarySubtractInst (:any) %15: any, 1: number
// CHKIR-NEXT:        ReturnInst %16: any
// CHKIR-NEXT:function_end

// CHKIR:arrow var1(): any [allCallsitesKnownInStrictMode]
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       StoreFrameInst 0: number, [var3@decrementArguments]: any
// CHKIR-NEXT:       ReturnInst 0: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DCE

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %2 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:any) 0: number, %BB0, %6: number|bigint, %BB1
// CHKIR-NEXT:       StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:  %6 = UnaryIncInst (:number|bigint) %4: any
// CHKIR-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: number|bigint, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %10 = BinarySubtractInst (:any) %9: any, 1: number
// CHKIR-NEXT:        ReturnInst %10: any
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER ObjectStackPromotion

// CHKIR:function global(): any
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): any
// CHKIR-NEXT:frame = [var3: any]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: any
// CHKIR-NEXT:  %2 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:any) 0: number, %BB0, %6: number|bigint, %BB1
// CHKIR-NEXT:       StoreFrameInst %0: object, [var3]: any
// CHKIR-NEXT:  %6 = UnaryIncInst (:number|bigint) %4: any
// CHKIR-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: number|bigint, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadFrameInst (:any) [var3]: any
// CHKIR-NEXT:  %10 = BinarySubtractInst (:any) %9: any, 1: number
// CHKIR-NEXT:        ReturnInst %10: any
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER TypeInference

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = [var3: undefined|object]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreFrameInst undefined: undefined, [var3]: undefined|object
// CHKIR-NEXT:  %2 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreFrameInst %0: object, [var3]: undefined|object
// CHKIR-NEXT:  %6 = UnaryIncInst (:number) %4: number
// CHKIR-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadFrameInst (:undefined|object) [var3]: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimpleStackPromotion

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:  %3 = BinaryLessThanInst (:boolean) 0: number, 2: number
// CHKIR-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %5 = PhiInst (:number) 0: number, %BB0, %7: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %7 = UnaryIncInst (:number) %5: number
// CHKIR-NEXT:  %8 = BinaryLessThanInst (:boolean) %7: number, 2: number
// CHKIR-NEXT:       CondBranchInst %8: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %10 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %11 = BinarySubtractInst (:number) %10: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %11: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER InstSimplify

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DCE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FunctionAnalysis

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Inlining

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimpleStackPromotion

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FrameLoadStoreOpts

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FunctionAnalysis

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %0: undefined|object
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHKIR-NEXT:       StoreStackInst %1: object, %0: undefined|object
// CHKIR-NEXT:  %6 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %7 = FLessThanInst (:boolean) %6: number, 2: number
// CHKIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Mem2Reg

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER ObjectStackPromotion

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER TypeInference

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER CSE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER TDZDedup

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       CondBranchInst true: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimplifyCFG

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = PhiInst (:object) %1: object, %BB1
// CHKIR-NEXT:  %9 = BinarySubtractInst (:number) %8: object, 1: number
// CHKIR-NEXT:        ReturnInst %9: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER InstSimplify

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %8: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FuncSigOpts

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $var3: any
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %1: object, %BB1
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %8 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %8: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DCE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SimplifyCFG

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER FrameLoadStoreOpts

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Mem2Reg

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER Auditor

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER TypeInference

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER HoistStartGenerator

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** INITIAL STATE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER PeepholeLowering

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = CreateFunctionInst (:object) %decrementArguments(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %3: number, %BB1
// CHKIR-NEXT:  %3 = FAddInst (:number) %2: number, 1: number
// CHKIR-NEXT:  %4 = FLessThanInst (:boolean) %3: number, 2: number
// CHKIR-NEXT:       CondBranchInst %4: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %6 = BinarySubtractInst (:number) %0: object, 1: number
// CHKIR-NEXT:       ReturnInst %6: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerLoadStoreFrameInst

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER OptEnvironmentInit

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerBuiltinCalls

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerCalls

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerNumericProperties

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerAllocObjectLiteral

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = CreateArgumentsLooseInst (:object)
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %4: number, %BB1
// CHKIR-NEXT:  %4 = FAddInst (:number) %3: number, 1: number
// CHKIR-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 2: number
// CHKIR-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:  %7 = BinarySubtractInst (:number) %1: object, 1: number
// CHKIR-NEXT:       ReturnInst %7: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerArgumentsArray

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %1: undefined|object
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LimitAllocArray

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %1: undefined|object
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DedupReifyArguments

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %1: undefined|object
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerSwitchIntoJumpTables

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %1: undefined|object
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SwitchLowering

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "decrementArguments": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:       StoreStackInst undefined: undefined, %1: undefined|object
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %4 = PhiInst (:number) 0: number, %BB0, %5: number, %BB1
// CHKIR-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHKIR-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 2: number
// CHKIR-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %9 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %10 = BinarySubtractInst (:number) %9: undefined|object, 1: number
// CHKIR-NEXT:        ReturnInst %10: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LoadConstants

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %2: undefined, %1: undefined|object
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %8: number, %BB1
// CHKIR-NEXT:  %7 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %8 = FAddInst (:number) %6: number, %7: number
// CHKIR-NEXT:  %9 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %8: number, %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %14 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %13: undefined|object, %14: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerAllocObject

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %2: undefined, %1: undefined|object
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %8: number, %BB1
// CHKIR-NEXT:  %7 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %8 = FAddInst (:number) %6: number, %7: number
// CHKIR-NEXT:  %9 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %8: number, %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %14 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %13: undefined|object, %14: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerCondBranch

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %0: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %2: undefined, %1: undefined|object
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %8: number, %BB1
// CHKIR-NEXT:  %7 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %8 = FAddInst (:number) %6: number, %7: number
// CHKIR-NEXT:  %9 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %8: number, %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %14 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %13: undefined|object, %14: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER CodeMotion

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %2: undefined, %1: undefined|object
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %6 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:number) %4: number, %BB0, %9: number, %BB1
// CHKIR-NEXT:  %9 = FAddInst (:number) %8: number, %5: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %9: number, %6: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %14 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %13: undefined|object, %14: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER CSE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %1 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %2: undefined, %1: undefined|object
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %6 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:number) %4: number, %BB0, %9: number, %BB1
// CHKIR-NEXT:  %9 = FAddInst (:number) %8: number, %5: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %9: number, %6: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %1: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %1: undefined|object
// CHKIR-NEXT:  %14 = BinarySubtractInst (:number) %13: undefined|object, %5: number
// CHKIR-NEXT:        ReturnInst %14: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER DCE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %1: undefined, %0: undefined|object
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %7 = PhiInst (:number) %3: number, %BB0, %8: number, %BB1
// CHKIR-NEXT:  %8 = FAddInst (:number) %7: number, %4: number
// CHKIR-NEXT:  %9 = FLessThanInst (:boolean) %8: number, %5: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %12 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %13 = BinarySubtractInst (:number) %12: undefined|object, %4: number
// CHKIR-NEXT:        ReturnInst %13: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER HoistStartGenerator

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %1: undefined, %0: undefined|object
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %7 = PhiInst (:number) %3: number, %BB0, %8: number, %BB1
// CHKIR-NEXT:  %8 = FAddInst (:number) %7: number, %4: number
// CHKIR-NEXT:  %9 = FLessThanInst (:boolean) %8: number, %5: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %12 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %13 = BinarySubtractInst (:number) %12: undefined|object, %4: number
// CHKIR-NEXT:        ReturnInst %13: number
// CHKIR-NEXT:function_end

// CHKIR:*** INITIAL STATE

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerStoreInstrs

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerCalls

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER MovElimination

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER RecreateCheapValues

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LoadConstantValueNumbering

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SpillRegisters

// CHKIR:function global(): undefined
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:       DeclareGlobalVarInst "decrementArguments": string
// CHKIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, %1: any
// CHKIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "decrementArguments": string
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       ReturnInst %5: undefined
// CHKIR-NEXT:function_end

// CHKIR:*** INITIAL STATE

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:       StoreStackInst %1: undefined, %0: undefined|object
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %6 = MovInst (:number) %3: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:number) %6: number, %BB0, %11: number, %BB1
// CHKIR-NEXT:  %9 = FAddInst (:number) %8: number, %4: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %9: number, %5: number
// CHKIR-NEXT:  %11 = MovInst (:number) %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %14 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %14: undefined|object, %4: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerStoreInstrs

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = MovInst (:undefined) %1: undefined
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %6 = MovInst (:number) %3: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:number) %6: number, %BB0, %11: number, %BB1
// CHKIR-NEXT:  %9 = FAddInst (:number) %8: number, %4: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %9: number, %5: number
// CHKIR-NEXT:  %11 = MovInst (:number) %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %14 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %14: undefined|object, %4: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LowerCalls

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = MovInst (:undefined) %1: undefined
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %6 = MovInst (:number) %3: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %8 = PhiInst (:number) %6: number, %BB0, %11: number, %BB1
// CHKIR-NEXT:  %9 = FAddInst (:number) %8: number, %4: number
// CHKIR-NEXT:  %10 = FLessThanInst (:boolean) %9: number, %5: number
// CHKIR-NEXT:  %11 = MovInst (:number) %9: number
// CHKIR-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %14 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %15 = BinarySubtractInst (:number) %14: undefined|object, %4: number
// CHKIR-NEXT:        ReturnInst %15: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER MovElimination

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %5 = MovInst (:number) %2: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %7 = PhiInst (:number) %5: number, %BB0, %10: number, %BB1
// CHKIR-NEXT:  %8 = FAddInst (:number) %7: number, %3: number
// CHKIR-NEXT:  %9 = FLessThanInst (:boolean) %8: number, %4: number
// CHKIR-NEXT:  %10 = MovInst (:number) %8: number
// CHKIR-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %13 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %14 = BinarySubtractInst (:number) %13: undefined|object, %3: number
// CHKIR-NEXT:        ReturnInst %14: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER RecreateCheapValues

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %9: number, %BB1
// CHKIR-NEXT:  %7 = FAddInst (:number) %6: number, %2: number
// CHKIR-NEXT:  %8 = FLessThanInst (:boolean) %7: number, %3: number
// CHKIR-NEXT:  %9 = MovInst (:number) %7: number
// CHKIR-NEXT:        CondBranchInst %8: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %12 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %13 = BinarySubtractInst (:number) %12: undefined|object, %2: number
// CHKIR-NEXT:        ReturnInst %13: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER LoadConstantValueNumbering

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %9: number, %BB1
// CHKIR-NEXT:  %7 = FAddInst (:number) %6: number, %2: number
// CHKIR-NEXT:  %8 = FLessThanInst (:boolean) %7: number, %3: number
// CHKIR-NEXT:  %9 = MovInst (:number) %7: number
// CHKIR-NEXT:        CondBranchInst %8: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %12 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %13 = BinarySubtractInst (:number) %12: undefined|object, %2: number
// CHKIR-NEXT:        ReturnInst %13: number
// CHKIR-NEXT:function_end

// CHKIR:*** AFTER SpillRegisters

// CHKIR:function decrementArguments(): number
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = AllocStackInst (:undefined|object) $arguments: any
// CHKIR-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKIR-NEXT:  %2 = HBCLoadConstInst (:number) 1: number
// CHKIR-NEXT:  %3 = HBCLoadConstInst (:number) 2: number
// CHKIR-NEXT:  %4 = HBCLoadConstInst (:number) 0: number
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %6 = PhiInst (:number) %4: number, %BB0, %9: number, %BB1
// CHKIR-NEXT:  %7 = FAddInst (:number) %6: number, %2: number
// CHKIR-NEXT:  %8 = FLessThanInst (:boolean) %7: number, %3: number
// CHKIR-NEXT:  %9 = MovInst (:number) %7: number
// CHKIR-NEXT:        CondBranchInst %8: boolean, %BB1, %BB2
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:        HBCReifyArgumentsLooseInst %0: undefined|object
// CHKIR-NEXT:  %12 = LoadStackInst (:undefined|object) %0: undefined|object
// CHKIR-NEXT:  %13 = BinarySubtractInst (:number) %12: undefined|object, %2: number
// CHKIR-NEXT:        ReturnInst %13: number
// CHKIR-NEXT:function_end

// CHKRA:function global(): undefined
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 = DeclareGlobalVarInst "decrementArguments": string
// CHKRA-NEXT:  $Reg0 = HBCCreateEnvironmentInst (:any)
// CHKRA-NEXT:  $Reg1 = HBCCreateFunctionInst (:object) %decrementArguments(): functionCode, $Reg0
// CHKRA-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHKRA-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "decrementArguments": string
// CHKRA-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end

// CHKRA:function decrementArguments(): number
// CHKRA-NEXT:frame = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  $Reg0 = AllocStackInst (:undefined|object) $arguments: any
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKRA-NEXT:  $Reg1 = StoreStackInst $Reg1, $Reg0
// CHKRA-NEXT:  $Reg4 = HBCLoadConstInst (:number) 0: number
// CHKRA-NEXT:  $Reg1 = HBCLoadConstInst (:number) 1: number
// CHKRA-NEXT:  $Reg3 = HBCLoadConstInst (:number) 2: number
// CHKRA-NEXT:  $Reg4 = MovInst (:number) $Reg4
// CHKRA-NEXT:  $Reg2 = BranchInst %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  $Reg4 = PhiInst (:number) $Reg4, %BB0, $Reg4, %BB1
// CHKRA-NEXT:  $Reg4 = FAddInst (:number) $Reg4, $Reg1
// CHKRA-NEXT:  $Reg2 = FLessThanInst (:boolean) $Reg4, $Reg3
// CHKRA-NEXT:  $Reg4 = MovInst (:number) $Reg4
// CHKRA-NEXT:  $Reg2 = CondBranchInst $Reg2, %BB1, %BB2
// CHKRA-NEXT:%BB2:
// CHKRA-NEXT:  $Reg2 = HBCReifyArgumentsLooseInst $Reg0
// CHKRA-NEXT:  $Reg0 = LoadStackInst (:undefined|object) $Reg0
// CHKRA-NEXT:  $Reg0 = BinarySubtractInst (:number) $Reg0, $Reg1
// CHKRA-NEXT:  $Reg0 = ReturnInst $Reg0
// CHKRA-NEXT:function_end
