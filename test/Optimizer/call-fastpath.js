/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -target=HBC -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(f, a, b) {
  f.call(a, b);
  f.call(b, a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(f: any, a: any, b: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %f: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %0: any, "call": string
// CHECK-NEXT:       BranchIfBuiltinInst [HermesBuiltin.functionPrototypeCall]: number, %3: any, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: any, "call": string
// CHECK-NEXT:       BranchIfBuiltinInst [HermesBuiltin.functionPrototypeCall]: number, %5: any, %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, %1: any, %2: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %1: any, %2: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: any, %1: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %14 = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %2: any, %1: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end
