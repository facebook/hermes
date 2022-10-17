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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [foo1, foo2, foo3, foo4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo1#0#1()#2 : number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo2#0#1()#4 : string|number, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "foo2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %foo3#0#1()#6, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "foo3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %foo4#0#1()#8, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "foo4" : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo1#0#1(a)#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo1#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst 100 : number
// CHECK-NEXT:function_end

// CHECK:function foo2#0#1(a)#4 : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo2#0#1()#4}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %a, 10 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : string|number
// CHECK-NEXT:function_end

// CHECK:function foo3#0#1(a)#6
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo3#0#1()#6}
// CHECK-NEXT:  %1 = CondBranchInst %a, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = PhiInst %a, %BB1, 10 : number, %BB0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function foo4#0#1(a)#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo4#0#1()#8}
// CHECK-NEXT:  %1 = BinaryOperatorInst '<', %a, 0 : number
// CHECK-NEXT:  %2 = CondBranchInst %1 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst 10 : number, %BB3, %a, %BB2, -1 : number, %BB0
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BinaryOperatorInst '==', %a, 0 : number
// CHECK-NEXT:  %6 = CondBranchInst %5 : boolean, %BB3, %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:function_end
