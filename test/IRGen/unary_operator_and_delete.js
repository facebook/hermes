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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "unary_operator_test": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "delete_test": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %unary_operator_test(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %delete_test(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "delete_test": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "unary_operator_test": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %10 = StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "delete_test": string
// CHECK-NEXT:  %12 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %13 = StoreStackInst %12: any, %6: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %15 = ReturnInst (:any) %14: any
// CHECK-NEXT:function_end

// CHECK:function unary_operator_test(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = AsNumberInst (:number) %2: any
// CHECK-NEXT:  %4 = ReturnInst (:any) %3: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = UnaryMinusInst (:any) %5: any
// CHECK-NEXT:  %7 = ReturnInst (:any) %6: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %9 = UnaryTildeInst (:any) %8: any
// CHECK-NEXT:  %10 = ReturnInst (:any) %9: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %12 = UnaryBangInst (:any) %11: any
// CHECK-NEXT:  %13 = ReturnInst (:any) %12: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %15 = UnaryTypeofInst (:any) %14: any
// CHECK-NEXT:  %16 = ReturnInst (:any) %15: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function delete_test(o: any): any
// CHECK-NEXT:frame = [o: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %3 = DeletePropertyLooseInst (:any) %2: any, "f": string
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %5 = DeletePropertyLooseInst (:any) %4: any, 3: number
// CHECK-NEXT:  %6 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
