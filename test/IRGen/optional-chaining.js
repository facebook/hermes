/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a) {
  return a?.b;
}

function f2(f) {
  return f?.();
}

function f3(a) {
  return a?.b.c;
}

function f4(a) {
  return a?.b().c;
}

function f5(a) {
  return a?.().b;
}

function f6(a) {
  return (a?.b.c).d;
}

function f7(a) {
  return a?.b?.().c;
}

function f8(a) {
  return a?.b?.c?.();
}

function f9(a) {
  return delete a?.b;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "f2" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "f3" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "f4" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "f5" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "f6" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "f7" : string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "f8" : string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "f9" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %f1()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %f2()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %f3()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "f3" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %f4()
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15 : closure, globalObject : object, "f4" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %f5()
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17 : closure, globalObject : object, "f5" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %f6()
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19 : closure, globalObject : object, "f6" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %f7()
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21 : closure, globalObject : object, "f7" : string
// CHECK-NEXT:  %23 = CreateFunctionInst %f8()
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23 : closure, globalObject : object, "f8" : string
// CHECK-NEXT:  %25 = CreateFunctionInst %f9()
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25 : closure, globalObject : object, "f9" : string
// CHECK-NEXT:  %27 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %28 = StoreStackInst undefined : undefined, %27
// CHECK-NEXT:  %29 = LoadStackInst %27
// CHECK-NEXT:  %30 = ReturnInst %29
// CHECK-NEXT:function_end

// CHECK:function f1(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2(f)
// CHECK-NEXT:frame = [f]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %f
// CHECK-NEXT:  %1 = StoreFrameInst %0, [f]
// CHECK-NEXT:  %2 = LoadFrameInst [f]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f3(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %9, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "c" : string
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f4(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %10, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = CallInst %8, %2
// CHECK-NEXT:  %10 = LoadPropertyInst %9, "c" : string
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f5(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %9, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "b" : string
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f6(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %10, %BB2
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "d" : string
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %10 = LoadPropertyInst %9, "c" : string
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f7(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %12, %BB4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = BinaryOperatorInst '==', %8, null : null
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = CallInst %8, %2
// CHECK-NEXT:  %12 = LoadPropertyInst %11, "c" : string
// CHECK-NEXT:  %13 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f8(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %14, %BB4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = BinaryOperatorInst '==', %8, null : null
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB1, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = LoadPropertyInst %8, "c" : string
// CHECK-NEXT:  %12 = BinaryOperatorInst '==', %11, null : null
// CHECK-NEXT:  %13 = CondBranchInst %12, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CallInst %11, %8
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f9(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = DeletePropertyLooseInst %2, "b" : string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
