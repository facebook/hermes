/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function foo(a, b = a) {
    return a + b;
}

function bar(a = 10, b = glob) {
    return a + b;
}

function baz({a, b}) {
    return a + b;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "bar" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "baz" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %bar()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %baz()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "baz" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function foo(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %2 = LoadParamInst %a
// CHECK-NEXT:  %3 = StoreFrameInst %2, [a]
// CHECK-NEXT:  %4 = LoadParamInst %b
// CHECK-NEXT:  %5 = BinaryOperatorInst '!==', %4, undefined : undefined
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst [a]
// CHECK-NEXT:  %8 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = PhiInst %4, %BB0, %7, %BB2
// CHECK-NEXT:  %10 = StoreFrameInst %9, [b]
// CHECK-NEXT:  %11 = LoadFrameInst [a]
// CHECK-NEXT:  %12 = LoadFrameInst [b]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %11, %12
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %2 = LoadParamInst %a
// CHECK-NEXT:  %3 = BinaryOperatorInst '!==', %2, undefined : undefined
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst %2, %BB0, 10 : number, %BB2
// CHECK-NEXT:  %7 = StoreFrameInst %6, [a]
// CHECK-NEXT:  %8 = LoadParamInst %b
// CHECK-NEXT:  %9 = BinaryOperatorInst '!==', %8, undefined : undefined
// CHECK-NEXT:  %10 = CondBranchInst %9, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "glob" : string
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = PhiInst %8, %BB1, %11, %BB4
// CHECK-NEXT:  %14 = StoreFrameInst %13, [b]
// CHECK-NEXT:  %15 = LoadFrameInst [a]
// CHECK-NEXT:  %16 = LoadFrameInst [b]
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %15, %16
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function baz(?anon_0_param)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %2 = LoadParamInst %?anon_0_param
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "a" : string
// CHECK-NEXT:  %4 = StoreFrameInst %3, [a]
// CHECK-NEXT:  %5 = LoadPropertyInst %2, "b" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [b]
// CHECK-NEXT:  %7 = LoadFrameInst [a]
// CHECK-NEXT:  %8 = LoadFrameInst [b]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %7, %8
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
