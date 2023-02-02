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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:18:2)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %4 = LoadFrameInst [a]
// CHECK-NEXT:; <stdin>:11:13
// CHECK-NEXT:  %5 = LoadFrameInst [b]
// CHECK-NEXT:; <stdin>:11:9
// CHECK-NEXT:  %6 = BinaryOperatorInst '>', %4, %5
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %7 = CondBranchInst %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:; <stdin>:12:9
// CHECK-NEXT:  %8 = LoadFrameInst [a]
// CHECK-NEXT:; <stdin>:12:14
// CHECK-NEXT:  %9 = LoadFrameInst [b]
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %10 = BinaryOperatorInst '-', %8, %9
// CHECK-NEXT:; <stdin>:12:11
// CHECK-NEXT:  %11 = StoreFrameInst %10, [a]
// CHECK-NEXT:; <stdin>:13:9
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:; <stdin>:13:15
// CHECK-NEXT:  %13 = LoadFrameInst [a]
// CHECK-NEXT:; <stdin>:13:14
// CHECK-NEXT:  %14 = CallInst %12, undefined : undefined, %13
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %15 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:; <stdin>:15:9
// CHECK-NEXT:  %16 = LoadFrameInst [b]
// CHECK-NEXT:; <stdin>:15:14
// CHECK-NEXT:  %17 = LoadFrameInst [a]
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %18 = BinaryOperatorInst '-', %16, %17
// CHECK-NEXT:; <stdin>:15:11
// CHECK-NEXT:  %19 = StoreFrameInst %18, [b]
// CHECK-NEXT:; <stdin>:16:9
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:; <stdin>:16:15
// CHECK-NEXT:  %21 = LoadFrameInst [b]
// CHECK-NEXT:; <stdin>:16:14
// CHECK-NEXT:  %22 = CallInst %20, undefined : undefined, %21
// CHECK-NEXT:; <stdin>:11:5
// CHECK-NEXT:  %23 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:; <stdin>:18:1
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
