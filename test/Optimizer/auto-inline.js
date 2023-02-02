/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo1(a) {
    var add = function() {
        return 100;
    }
    return add(a, 10);
}

function foo2(a) {
    var add = function(a, b) {
        return a + b;
    }
    return add(a, 10);
}

function foo3(a) {
    var add = function(a, b) {
        return a ? a : b;
    }
    return add(a, 10);
}

function foo4(a) {
    var add = function(a, b) {
        if (a < 0)
            return -1;
        if (a == 0)
            return b;
        return a;
    }
    return add(a, 10);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "foo2" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "foo3" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "foo4" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %foo1() : number
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "foo1" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %foo2() : string|number
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "foo2" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %foo3()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "foo3" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %foo4()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "foo4" : string
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo1(a) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 100 : number
// CHECK-NEXT:function_end

// CHECK:function foo2(a) : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %0, 10 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : string|number
// CHECK-NEXT:function_end

// CHECK:function foo3(a)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = CondBranchInst %0, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = PhiInst %0, %BB1, 10 : number, %BB0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function foo4(a)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = BinaryOperatorInst '<', %0, 0 : number
// CHECK-NEXT:  %2 = CondBranchInst %1 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst 10 : number, %BB3, %0, %BB2, -1 : number, %BB0
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BinaryOperatorInst '==', %0, 0 : number
// CHECK-NEXT:  %6 = CondBranchInst %5 : boolean, %BB3, %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:function_end
