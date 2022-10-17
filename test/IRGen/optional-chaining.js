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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [f1, f2, f3, f4, f5, f6, f7, f8, f9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %f1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %f2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %f3#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "f3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %f4#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "f4" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %f5#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "f5" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %f6#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "f6" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %f7#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "f7" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %f8#0#1()#9, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "f8" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %f9#0#1()#10, %0
// CHECK-NEXT:  %18 = StorePropertyInst %17 : closure, globalObject : object, "f9" : string
// CHECK-NEXT:  %19 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %20 = StoreStackInst undefined : undefined, %19
// CHECK-NEXT:  %21 = LoadStackInst %19
// CHECK-NEXT:  %22 = ReturnInst %21
// CHECK-NEXT:function_end

// CHECK:function f1#0#1(a)#2
// CHECK-NEXT:frame = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#2], %0
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

// CHECK:function f2#0#1(f)#3
// CHECK-NEXT:frame = [f#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %f, [f#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [f#3], %0
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

// CHECK:function f3#0#1(a)#4
// CHECK-NEXT:frame = [a#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f3#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#4], %0
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

// CHECK:function f4#0#1(a)#5
// CHECK-NEXT:frame = [a#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f4#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#5], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#5], %0
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

// CHECK:function f5#0#1(a)#6
// CHECK-NEXT:frame = [a#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f5#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#6], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#6], %0
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

// CHECK:function f6#0#1(a)#7
// CHECK-NEXT:frame = [a#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f6#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#7], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#7], %0
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

// CHECK:function f7#0#1(a)#8
// CHECK-NEXT:frame = [a#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f7#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#8], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#8], %0
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

// CHECK:function f8#0#1(a)#9
// CHECK-NEXT:frame = [a#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f8#0#1()#9}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#9], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#9], %0
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

// CHECK:function f9#0#1(a)#10
// CHECK-NEXT:frame = [a#10]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f9#0#1()#10}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#10], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#10], %0
// CHECK-NEXT:  %3 = BinaryOperatorInst '==', %2, null : null
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %5 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = DeletePropertyInst %2, "b" : string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
