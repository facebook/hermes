/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes - -O0 -dump-source-location=loc -dump-ir < %s | %FileCheck --match-full-lines %s

function foo(a,b) {
    if (a > b) {
        a -= b;
        print(a);
    } else {
        b -= a;
        print(b);
    }
}

//CHECK-LABEL: function global()
//CHECK-NEXT: frame = [], globals = [foo]
//CHECK-NEXT: source location: [<stdin>:10:1 ... <stdin>:18:2)
//CHECK-NEXT: %BB0:
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %0 = CreateFunctionInst %foo()
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %1 = StorePropertyInst %0 : closure, globalObject : object, "foo" : string
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %2 = AllocStackInst $?anon_0_ret
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %3 = StoreStackInst undefined : undefined, %2
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %4 = LoadStackInst %2
//CHECK-NEXT: ; <stdin>:18:1
//CHECK-NEXT:   %5 = ReturnInst %4
//CHECK-NEXT: function_end

//CHECK-LABEL: function foo(a, b)
//CHECK-NEXT: frame = [a, b]
//CHECK-NEXT: source location: [<stdin>:10:1 ... <stdin>:18:2)
//CHECK-NEXT: %BB0:
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
//CHECK-NEXT: ; <stdin>:10:1
//CHECK-NEXT:   %1 = StoreFrameInst %b, [b]
//CHECK-NEXT: ; <stdin>:11:9
//CHECK-NEXT:   %2 = LoadFrameInst [a]
//CHECK-NEXT: ; <stdin>:11:13
//CHECK-NEXT:   %3 = LoadFrameInst [b]
//CHECK-NEXT: ; <stdin>:11:9
//CHECK-NEXT:   %4 = BinaryOperatorInst '>', %2, %3
//CHECK-NEXT: ; <stdin>:11:5
//CHECK-NEXT:   %5 = CondBranchInst %4, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT: ; <stdin>:12:9
//CHECK-NEXT:   %6 = LoadFrameInst [a]
//CHECK-NEXT: ; <stdin>:12:14
//CHECK-NEXT:   %7 = LoadFrameInst [b]
//CHECK-NEXT: ; <stdin>:12:11
//CHECK-NEXT:   %8 = BinaryOperatorInst '-', %6, %7
//CHECK-NEXT: ; <stdin>:12:11
//CHECK-NEXT:   %9 = StoreFrameInst %8, [a]
//CHECK-NEXT: ; <stdin>:13:9
//CHECK-NEXT:   %10 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT: ; <stdin>:13:15
//CHECK-NEXT:   %11 = LoadFrameInst [a]
//CHECK-NEXT: ; <stdin>:13:14
//CHECK-NEXT:   %12 = CallInst %10, undefined : undefined, %11
//CHECK-NEXT: ; <stdin>:11:5
//CHECK-NEXT:   %13 = BranchInst %BB3
//CHECK-NEXT: %BB2:
//CHECK-NEXT: ; <stdin>:15:9
//CHECK-NEXT:   %14 = LoadFrameInst [b]
//CHECK-NEXT: ; <stdin>:15:14
//CHECK-NEXT:   %15 = LoadFrameInst [a]
//CHECK-NEXT: ; <stdin>:15:11
//CHECK-NEXT:   %16 = BinaryOperatorInst '-', %14, %15
//CHECK-NEXT: ; <stdin>:15:11
//CHECK-NEXT:   %17 = StoreFrameInst %16, [b]
//CHECK-NEXT: ; <stdin>:16:9
//CHECK-NEXT:   %18 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT: ; <stdin>:16:15
//CHECK-NEXT:   %19 = LoadFrameInst [b]
//CHECK-NEXT: ; <stdin>:16:14
//CHECK-NEXT:   %20 = CallInst %18, undefined : undefined, %19
//CHECK-NEXT: ; <stdin>:11:5
//CHECK-NEXT:   %21 = BranchInst %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT: ; <stdin>:18:1
//CHECK-NEXT:   %22 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
