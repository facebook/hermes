/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function unary_operator_test(x) {
  return +x;
  return -x;
  return ~x;
  return !x;
  return typeof x;
}

function delete_test(o) {
  delete o;
  delete o.f;
  delete o[3];
}

unary_operator_test()
delete_test()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "unary_operator_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_test": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %unary_operator_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %delete_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "delete_test": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) globalObject: object, "delete_test": string
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %13: any, %7: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function unary_operator_test(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %unary_operator_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = AsNumberInst (:number) %4: any
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function delete_test(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %delete_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %5 = DeletePropertyLooseInst (:any) %4: any, "f": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %7 = DeletePropertyLooseInst (:any) %6: any, 3: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
