/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

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
// CHECK-NEXT:frame = [], globals = [foo, bar, baz]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %bar()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %baz()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "baz" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function foo(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = BinaryOperatorInst '!==', %2, undefined : undefined
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst [a]
// CHECK-NEXT:  %6 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst %2, %BB0, %5, %BB2
// CHECK-NEXT:  %8 = StoreFrameInst %7, [b]
// CHECK-NEXT:  %9 = LoadFrameInst [a]
// CHECK-NEXT:  %10 = LoadFrameInst [b]
// CHECK-NEXT:  %11 = BinaryOperatorInst '+', %9, %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %0, undefined : undefined
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst %0, %BB0, 10 : number, %BB2
// CHECK-NEXT:  %5 = StoreFrameInst %4, [a]
// CHECK-NEXT:  %6 = LoadParamInst %b
// CHECK-NEXT:  %7 = BinaryOperatorInst '!==', %6, undefined : undefined
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "glob" : string
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = PhiInst %6, %BB1, %9, %BB4
// CHECK-NEXT:  %12 = StoreFrameInst %11, [b]
// CHECK-NEXT:  %13 = LoadFrameInst [a]
// CHECK-NEXT:  %14 = LoadFrameInst [b]
// CHECK-NEXT:  %15 = BinaryOperatorInst '+', %13, %14
// CHECK-NEXT:  %16 = ReturnInst %15
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function baz(?anon_0_param)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %?anon_0_param
// CHECK-NEXT:  %1 = LoadPropertyInst %0, "a" : string
// CHECK-NEXT:  %2 = StoreFrameInst %1, [a]
// CHECK-NEXT:  %3 = LoadPropertyInst %0, "b" : string
// CHECK-NEXT:  %4 = StoreFrameInst %3, [b]
// CHECK-NEXT:  %5 = LoadFrameInst [a]
// CHECK-NEXT:  %6 = LoadFrameInst [b]
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
