/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
    return eval("1 + 1");
}

function bar() {
    return eval("2 + 2", Math, foo());
}

function baz() {
    return eval();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "baz": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %baz(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "baz": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "eval": string
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [globalThis.eval]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %2: any, %3: object, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = DirectEvalInst (:any) "1 + 1": string, false: boolean
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "1 + 1": string
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = PhiInst (:any) %5: any, %BB1, %7: any, %BB2
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function bar(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %bar(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "eval": string
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = GetBuiltinClosureInst (:object) [globalThis.eval]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %2: any, %6: object, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = DirectEvalInst (:any) "2 + 2": string, false: boolean
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "2 + 2": string, %3: any, %5: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = PhiInst (:any) %8: any, %BB1, %10: any, %BB2
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function baz(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %baz(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "eval": string
// CHECK-NEXT:  %3 = GetBuiltinClosureInst (:object) [globalThis.eval]: number
// CHECK-NEXT:       CmpBrStrictlyEqualInst %2: any, %3: object, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = DirectEvalInst (:any) undefined: undefined, false: boolean
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = PhiInst (:any) %5: any, %BB1, %7: any, %BB2
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end
