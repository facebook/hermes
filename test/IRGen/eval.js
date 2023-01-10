/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
    return eval("1 + 1");
}

function bar() {
    return eval("2 + 2", Math, foo());
}

function baz() {
    return eval();
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

// CHECK:function foo()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "eval" : string
// CHECK-NEXT:  %1 = GetBuiltinClosureInst [globalThis.eval] : number
// CHECK-NEXT:  %2 = CompareBranchInst '===', %0, %1 : closure, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = DirectEvalInst "1 + 1" : string, false : boolean
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, "1 + 1" : string
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst %3, %BB1, %5, %BB2
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "eval" : string
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "Math" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = GetBuiltinClosureInst [globalThis.eval] : number
// CHECK-NEXT:  %5 = CompareBranchInst '===', %0, %4 : closure, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = DirectEvalInst "2 + 2" : string, false : boolean
// CHECK-NEXT:  %7 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, "2 + 2" : string, %1, %3
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = PhiInst %6, %BB1, %8, %BB2
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function baz()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "eval" : string
// CHECK-NEXT:  %1 = GetBuiltinClosureInst [globalThis.eval] : number
// CHECK-NEXT:  %2 = CompareBranchInst '===', %0, %1 : closure, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = DirectEvalInst undefined : undefined, false : boolean
// CHECK-NEXT:  %4 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = PhiInst %3, %BB1, %5, %BB2
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
