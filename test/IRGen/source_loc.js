/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes - -O0 -dump-source-location=loc -dump-ir < %s | %FileCheckOrRegen --match-full-lines %s

function foo(a,b) {
    if (a > b) {
        a -= b;
        print(a);
    } else {
        b -= a;
        print(b);
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b)#2
// CHECK-NEXT:frame = [a#2, b#2]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %3 = LoadFrameInst [a#2], %0
// CHECK-NEXT:; <stdin>:11:13
// CHECK-NEXT:  %4 = LoadFrameInst [b#2], %0
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %5 = BinaryOperatorInst '>', %3, %4
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:; <stdin>:12:9
// CHECK-NEXT:  %7 = LoadFrameInst [a#2], %0
// CHECK-NEXT:; <stdin>:12:14
// CHECK-NEXT:  %8 = LoadFrameInst [b#2], %0
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %9 = BinaryOperatorInst '-', %7, %8
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %10 = StoreFrameInst %9, [a#2], %0
// CHECK-NEXT:; <stdin>:13:9
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:; <stdin>:13:15
// CHECK-NEXT:  %12 = LoadFrameInst [a#2], %0
// CHECK-NEXT:; <stdin>:13:14
// CHECK-NEXT:  %13 = CallInst %11, undefined : undefined, %12
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %14 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:; <stdin>:15:9
// CHECK-NEXT:  %15 = LoadFrameInst [b#2], %0
// CHECK-NEXT:; <stdin>:15:14
// CHECK-NEXT:  %16 = LoadFrameInst [a#2], %0
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %17 = BinaryOperatorInst '-', %15, %16
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %18 = StoreFrameInst %17, [b#2], %0
// CHECK-NEXT:; <stdin>:16:9
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:; <stdin>:16:15
// CHECK-NEXT:  %20 = LoadFrameInst [b#2], %0
// CHECK-NEXT:; <stdin>:16:14
// CHECK-NEXT:  %21 = CallInst %19, undefined : undefined, %20
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %22 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:  %23 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
