/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s

function foo(dim) {
  var a = (dim == dim); // This creates a bool.
  var b = (dim == dim); // This creates a bool.
  var c = a + b;
  var d = a + b;
  return c * d;
}

// This is supposed to be a CSE across blocks, but LoadStoreOpts is
// not across basic blocks, we end up loading some values for the BinaryOperator.
function foo_with_cf(dim) {
  var d = 0;
  var a = (dim == dim);
  var b = (dim == dim);
  var c = a + b;
  if (a) {
    d = a + b;
  }
  return c * d;
}

// Make sure we are not merging the plus and minus together.
function check_operator_kind(i) {
  var x = i | 0;
  var y = i | 0;
  var t0 = x - y;
  var t1 = x + y;
  return t0 * t1;
}

function cse_this_instr() {
  print(this, this, this, this)
}

function cse_unary(a) {
    ++a;
    return (-a) * (-a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_with_cf": string
// CHECK-NEXT:       DeclareGlobalVarInst "check_operator_kind": string
// CHECK-NEXT:       DeclareGlobalVarInst "cse_this_instr": string
// CHECK-NEXT:       DeclareGlobalVarInst "cse_unary": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "foo": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %foo_with_cf(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "foo_with_cf": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %check_operator_kind(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "check_operator_kind": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %cse_this_instr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "cse_this_instr": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %cse_unary(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "cse_unary": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(dim: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %dim: any
// CHECK-NEXT:  %1 = BinaryEqualInst (:boolean) %0: any, %0: any
// CHECK-NEXT:  %2 = BinaryEqualInst (:boolean) %0: any, %0: any
// CHECK-NEXT:  %3 = BinaryAddInst (:number) %1: boolean, %2: boolean
// CHECK-NEXT:  %4 = FMultiplyInst (:number) %3: number, %3: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:function foo_with_cf(dim: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %dim: any
// CHECK-NEXT:  %1 = BinaryEqualInst (:boolean) %0: any, %0: any
// CHECK-NEXT:  %2 = BinaryEqualInst (:boolean) %0: any, %0: any
// CHECK-NEXT:  %3 = BinaryAddInst (:number) %1: boolean, %2: boolean
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = PhiInst (:number) %3: number, %BB1, 0: number, %BB0
// CHECK-NEXT:  %7 = FMultiplyInst (:number) %3: number, %6: number
// CHECK-NEXT:       ReturnInst %7: number
// CHECK-NEXT:function_end

// CHECK:function check_operator_kind(i: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %i: any
// CHECK-NEXT:  %1 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %2 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %3 = FSubtractInst (:number) %1: number, %2: number
// CHECK-NEXT:  %4 = FAddInst (:number) %1: number, %2: number
// CHECK-NEXT:  %5 = FMultiplyInst (:number) %3: number, %4: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function cse_this_instr(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: object, %1: object, %1: object, %1: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function cse_unary(a: any): number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = UnaryIncInst (:number|bigint) %0: any
// CHECK-NEXT:  %2 = UnaryMinusInst (:number|bigint) %1: number|bigint
// CHECK-NEXT:  %3 = BinaryMultiplyInst (:number|bigint) %2: number|bigint, %2: number|bigint
// CHECK-NEXT:       ReturnInst %3: number|bigint
// CHECK-NEXT:function_end
