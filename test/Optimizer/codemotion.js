/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function hoist_branch(x, y) {
  if (x) {
    return y.z.k.r;
  } else {
    return y.z.k.t;
  }
}

function hoist_branch_window(x, y) {
  var n = y | 0;

  if (x) {
    var z = n + n;
    return y.z.k.r + z;
  } else {
    return y.z.k.t;
  }
}

function no_hoist_inc_dec(x, y) {
  if (x) {
    y++;
  } else {
    y--;
  }
  return y;
}

function hoist_loop(x) {
  for (var i = 0; i < x; i++) {
    print(i);
  }
}

function hoist_loop_expression(a, b, call) {
  a = +a;
  b = +b;
  for (;;) {
    call(-a * (b + 7));
  }
}

function hoist_from_multiblock_loop(x) {
  x = +x;
  for (;;) {
    print(3 * x * x);
    if (x - 1) {
      print(3 * x * x);
    }
  }
}

function hoist_with_dependencies_in_loop(x, y) {
  x = +x;
  for (;;) {
    var x2 = x * x;
    if (y) {
      return y;
    }
    print(x2 - 3);
  }
}

function code_sinking(x, y) {
  var n = (y | 0) + 2;
  var k = (n) + 9;

  if (x) {
    return n;
  } else {
    return k;
  }
}

function code_sinking_in_loop(x, y) {
  for (;;) {
    var obj = {};
    if (y) {
      return;
    }
    obj[x] = y;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_branch": string
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_branch_window": string
// CHECK-NEXT:       DeclareGlobalVarInst "no_hoist_inc_dec": string
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_loop_expression": string
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_from_multiblock_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "hoist_with_dependencies_in_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "code_sinking": string
// CHECK-NEXT:       DeclareGlobalVarInst "code_sinking_in_loop": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %hoist_branch(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "hoist_branch": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %hoist_branch_window(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "hoist_branch_window": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %no_hoist_inc_dec(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "no_hoist_inc_dec": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %hoist_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "hoist_loop": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %hoist_loop_expression(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "hoist_loop_expression": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %hoist_from_multiblock_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "hoist_from_multiblock_loop": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %hoist_with_dependencies_in_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "hoist_with_dependencies_in_loop": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %code_sinking(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %24: object, globalObject: object, "code_sinking": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %0: environment, %code_sinking_in_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %26: object, globalObject: object, "code_sinking_in_loop": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_branch(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "k": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "r": string
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "k": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "t": string
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function hoist_branch_window(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsInt32Inst (:number) %1: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = FAddInst (:number) %2: number, %2: number
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "k": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "r": string
// CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %7: any, %4: number
// CHECK-NEXT:       ReturnInst %8: string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %10: any, "k": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "t": string
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function no_hoist_inc_dec(x: any, y: any): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = UnaryIncInst (:number|bigint) %1: any
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = UnaryDecInst (:number|bigint) %1: any
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst (:number|bigint) %3: number|bigint, %BB1, %5: number|bigint, %BB2
// CHECK-NEXT:       ReturnInst %7: number|bigint
// CHECK-NEXT:function_end

// CHECK:function hoist_loop(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = BinaryLessThanInst (:boolean) 0: number, %0: any
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: number
// CHECK-NEXT:  %6 = FAddInst (:number) %3: number, 1: number
// CHECK-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: number, %0: any
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_loop_expression(a: any, b: any, call: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %call: any
// CHECK-NEXT:  %3 = AsNumberInst (:number) %0: any
// CHECK-NEXT:  %4 = AsNumberInst (:number) %1: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = FNegate (:number) %3: number
// CHECK-NEXT:  %7 = FAddInst (:number) %4: number, 7: number
// CHECK-NEXT:  %8 = FMultiplyInst (:number) %6: number, %7: number
// CHECK-NEXT:  %9 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_from_multiblock_loop(x: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AsNumberInst (:number) %0: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = FMultiplyInst (:number) 3: number, %1: number
// CHECK-NEXT:  %5 = FMultiplyInst (:number) %4: number, %1: number
// CHECK-NEXT:  %6 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: number
// CHECK-NEXT:  %7 = FSubtractInst (:number) %1: number, 1: number
// CHECK-NEXT:       CondBranchInst %7: number, %BB2, %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_with_dependencies_in_loop(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsNumberInst (:number) %0: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = FMultiplyInst (:number) %2: number, %2: number
// CHECK-NEXT:       CondBranchInst %1: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = FSubtractInst (:number) %4: number, 3: number
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function code_sinking(x: any, y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsInt32Inst (:number) %1: any
// CHECK-NEXT:  %3 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:  %4 = FAddInst (:number) %3: number, 9: number
// CHECK-NEXT:       CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:function code_sinking_in_loop(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       CondBranchInst %1: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       StorePropertyLooseInst %1: any, %3: object, %0: any
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
