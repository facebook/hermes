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
// CHECK-NEXT:       DeclareGlobalVarInst "unary_operator_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_test": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %unary_operator_test(): any
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %delete_test(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "delete_test": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "delete_test": string
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %12: any, %6: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function unary_operator_test(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = AsNumberInst (:number) %2: any
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function delete_test(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = DeletePropertyLooseInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %5 = DeletePropertyLooseInst (:any) %4: any, 3: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
