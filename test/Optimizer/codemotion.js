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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "hoist_branch": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "hoist_branch_window": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "no_hoist_inc_dec": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "hoist_loop": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "hoist_loop_expression": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "hoist_from_multiblock_loop": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "hoist_with_dependencies_in_loop": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "code_sinking": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "code_sinking_in_loop": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %hoist_branch(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "hoist_branch": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %hoist_branch_window(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "hoist_branch_window": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:closure) %no_hoist_inc_dec(): number|bigint
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: closure, globalObject: object, "no_hoist_inc_dec": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:closure) %hoist_loop(): undefined
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: closure, globalObject: object, "hoist_loop": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:closure) %hoist_loop_expression(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: closure, globalObject: object, "hoist_loop_expression": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:closure) %hoist_from_multiblock_loop(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: closure, globalObject: object, "hoist_from_multiblock_loop": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:closure) %hoist_with_dependencies_in_loop(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: closure, globalObject: object, "hoist_with_dependencies_in_loop": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:closure) %code_sinking(): number
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: closure, globalObject: object, "code_sinking": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:closure) %code_sinking_in_loop(): undefined
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: closure, globalObject: object, "code_sinking_in_loop": string
// CHECK-NEXT:  %27 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_branch(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "k": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "r": string
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "k": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "t": string
// CHECK-NEXT:  %10 = ReturnInst (:any) %9: any
// CHECK-NEXT:function_end

// CHECK:function hoist_branch_window(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsInt32Inst (:number) %1: any
// CHECK-NEXT:  %3 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BinaryAddInst (:number) %2: number, %2: number
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "k": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "r": string
// CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %7: any, %4: number
// CHECK-NEXT:  %9 = ReturnInst (:string|number) %8: string|number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %1: any, "z": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %10: any, "k": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "t": string
// CHECK-NEXT:  %13 = ReturnInst (:any) %12: any
// CHECK-NEXT:function_end

// CHECK:function no_hoist_inc_dec(x: any, y: any): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = UnaryIncInst (:number|bigint) %1: any
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = UnaryDecInst (:number|bigint) %1: any
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst (:number|bigint) %3: number|bigint, %BB1, %5: number|bigint, %BB2
// CHECK-NEXT:  %8 = ReturnInst (:number|bigint) %7: number|bigint
// CHECK-NEXT:function_end

// CHECK:function hoist_loop(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = BinaryLessThanInst (:boolean) 0: number, %0: any
// CHECK-NEXT:  %2 = CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %6: number, %BB1
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, %3: number
// CHECK-NEXT:  %6 = UnaryIncInst (:number) %3: number
// CHECK-NEXT:  %7 = BinaryLessThanInst (:boolean) %6: number, %0: any
// CHECK-NEXT:  %8 = CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function hoist_loop_expression(a: any, b: any, call: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %call: any
// CHECK-NEXT:  %3 = AsNumberInst (:number) %0: any
// CHECK-NEXT:  %4 = AsNumberInst (:number) %1: any
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = UnaryMinusInst (:number) %3: number
// CHECK-NEXT:  %7 = BinaryAddInst (:number) %4: number, 7: number
// CHECK-NEXT:  %8 = BinaryMultiplyInst (:number) %6: number, %7: number
// CHECK-NEXT:  %9 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %8: number
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_from_multiblock_loop(x: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AsNumberInst (:number) %0: any
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:number) 3: number, %1: number
// CHECK-NEXT:  %5 = BinaryMultiplyInst (:number) %4: number, %1: number
// CHECK-NEXT:  %6 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, %5: number
// CHECK-NEXT:  %7 = BinarySubtractInst (:number) %1: number, 1: number
// CHECK-NEXT:  %8 = CondBranchInst %7: number, %BB2, %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, %5: number
// CHECK-NEXT:  %11 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function hoist_with_dependencies_in_loop(x: any, y: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsNumberInst (:number) %0: any
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:number) %2: number, %2: number
// CHECK-NEXT:  %5 = CondBranchInst %1: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst (:any) %1: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = BinarySubtractInst (:number) %4: number, 3: number
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %8: number
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:function code_sinking(x: any, y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = AsInt32Inst (:number) %1: any
// CHECK-NEXT:  %3 = BinaryAddInst (:number) %2: number, 2: number
// CHECK-NEXT:  %4 = BinaryAddInst (:number) %3: number, 9: number
// CHECK-NEXT:  %5 = CondBranchInst %0: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst (:number) %3: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = ReturnInst (:number) %4: number
// CHECK-NEXT:function_end

// CHECK:function code_sinking_in_loop(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CondBranchInst %1: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = StorePropertyLooseInst %1: any, %3: object, %0: any
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:function_end
