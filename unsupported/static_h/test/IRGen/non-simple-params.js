/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck %s --match-full-lines

function foo(a, b = a) {
    return a + b;
}
//CHECK-LABEL: function foo(a, b)
//CHECK-NEXT: frame = [a, b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
//CHECK-NEXT:   %1 = BinaryOperatorInst '!==', %b, undefined : undefined
//CHECK-NEXT:   %2 = CondBranchInst %1, %BB1, %BB2
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %3 = LoadFrameInst [a]
//CHECK-NEXT:   %4 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %5 = PhiInst %b, %BB0, %3, %BB2
//CHECK-NEXT:   %6 = StoreFrameInst %5, [b]

function bar(a = 10, b = glob) {
    return a + b;
}
//CHECK-LABEL: function bar(a, b)
//CHECK-NEXT: frame = [a, b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = BinaryOperatorInst '!==', %a, undefined : undefined
//CHECK-NEXT:   %1 = CondBranchInst %0, %BB1, %BB2
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %2 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %3 = PhiInst %a, %BB0, 10 : number, %BB2
//CHECK-NEXT:   %4 = StoreFrameInst %3, [a]
//CHECK-NEXT:   %5 = BinaryOperatorInst '!==', %b, undefined : undefined
//CHECK-NEXT:   %6 = CondBranchInst %5, %BB3, %BB4
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %7 = TryLoadGlobalPropertyInst globalObject : object, "glob" : string
//CHECK-NEXT:   %8 = BranchInst %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %9 = PhiInst %b, %BB1, %7, %BB4
//CHECK-NEXT:   %10 = StoreFrameInst %9, [b]

function baz({a, b}) {
    return a + b;
}
//CHECK-LABEL: function baz(?anon_0_param)
//CHECK-NEXT: frame = [a, b]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = LoadPropertyInst %?anon_0_param, "a" : string
//CHECK-NEXT:   %1 = StoreFrameInst %0, [a]
//CHECK-NEXT:   %2 = LoadPropertyInst %?anon_0_param, "b" : string
//CHECK-NEXT:   %3 = StoreFrameInst %2, [b]
