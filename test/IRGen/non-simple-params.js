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
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = BinaryOperatorInst '!==', %b, undefined : undefined
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = LoadFrameInst [a]
// CHECK-NEXT:  %4 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = PhiInst %b, %BB0, %3, %BB2
// CHECK-NEXT:  %6 = StoreFrameInst %5, [b]
// CHECK-NEXT:  %7 = LoadFrameInst [a]
// CHECK-NEXT:  %8 = LoadFrameInst [b]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %7, %8
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '!==', %a, undefined : undefined
// CHECK-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst %a, %BB0, 10 : number, %BB2
// CHECK-NEXT:  %4 = StoreFrameInst %3, [a]
// CHECK-NEXT:  %5 = BinaryOperatorInst '!==', %b, undefined : undefined
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "glob" : string
// CHECK-NEXT:  %8 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = PhiInst %b, %BB1, %7, %BB4
// CHECK-NEXT:  %10 = StoreFrameInst %9, [b]
// CHECK-NEXT:  %11 = LoadFrameInst [a]
// CHECK-NEXT:  %12 = LoadFrameInst [b]
// CHECK-NEXT:  %13 = BinaryOperatorInst '+', %11, %12
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function baz(?anon_0_param)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst %?anon_0_param, "a" : string
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadPropertyInst %?anon_0_param, "b" : string
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadFrameInst [a]
// CHECK-NEXT:  %5 = LoadFrameInst [b]
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
