/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
//
// Make sure that the optional initialization generates a correct Phi node, when the initialization
// is multi-block.

var a, b;
function foo(param = a || b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "foo": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %5: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function foo(param: any): any
// CHECK-NEXT:frame = [param: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [param]: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %param: any
// CHECK-NEXT:  %2 = BinaryStrictlyNotEqualInst (:any) %1: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %2: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       StoreStackInst %5: any, %4: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB4, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = PhiInst (:any) %1: any, %BB0, %14: any, %BB4
// CHECK-NEXT:       StoreFrameInst %8: any, [param]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:        StoreStackInst %11: any, %4: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
