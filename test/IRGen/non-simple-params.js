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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo, bar, baz]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %bar#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %baz#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "baz" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = LoadStackInst %7
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b)#2
// CHECK-NEXT:frame = [a#2, b#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = BinaryOperatorInst '!==', %b, undefined : undefined
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = PhiInst %b, %BB0, %4, %BB2
// CHECK-NEXT:  %7 = StoreFrameInst %6, [b#2], %0
// CHECK-NEXT:  %8 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %9 = LoadFrameInst [b#2], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %8, %9
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar#0#1(a, b)#3
// CHECK-NEXT:frame = [a#3, b#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#0#1()#3}
// CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %a, undefined : undefined
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst %a, %BB0, 10 : number, %BB2
// CHECK-NEXT:  %5 = StoreFrameInst %4, [a#3], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '!==', %b, undefined : undefined
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "glob" : string
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = PhiInst %b, %BB1, %8, %BB4
// CHECK-NEXT:  %11 = StoreFrameInst %10, [b#3], %0
// CHECK-NEXT:  %12 = LoadFrameInst [a#3], %0
// CHECK-NEXT:  %13 = LoadFrameInst [b#3], %0
// CHECK-NEXT:  %14 = BinaryOperatorInst '+', %12, %13
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function baz#0#1(?anon_0_param)#4
// CHECK-NEXT:frame = [a#4, b#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{baz#0#1()#4}
// CHECK-NEXT:  %1 = LoadPropertyInst %?anon_0_param, "a" : string
// CHECK-NEXT:  %2 = StoreFrameInst %1, [a#4], %0
// CHECK-NEXT:  %3 = LoadPropertyInst %?anon_0_param, "b" : string
// CHECK-NEXT:  %4 = StoreFrameInst %3, [b#4], %0
// CHECK-NEXT:  %5 = LoadFrameInst [a#4], %0
// CHECK-NEXT:  %6 = LoadFrameInst [b#4], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
