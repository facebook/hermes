/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -hermes-parser -dump-ir %s -O

var x = {
    1: 10,
    get a() { return "a" },
    get 1() { return 20; },
    b: 11,
    set 1(x) {},
    get 1() { return 21; },
    b: 12,
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "x" : string
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = AllocObjectInst 3 : number, empty
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst null : null, %3 : object, "1" : string, true : boolean
// CHECK-NEXT:  %5 = CreateFunctionInst %"get a"()
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5 : closure, undefined : undefined, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = CreateFunctionInst %"get 1"()
// CHECK-NEXT:  %8 = CreateFunctionInst %"set 1"()
// CHECK-NEXT:  %9 = StoreGetterSetterInst %7 : closure, %8 : closure, %3 : object, "1" : string, true : boolean
// CHECK-NEXT:  %10 = StoreNewOwnPropertyInst null : null, %3 : object, "b" : string, true : boolean
// CHECK-NEXT:  %11 = StoreOwnPropertyInst 12 : number, %3 : object, "b" : string, true : boolean
// CHECK-NEXT:  %12 = StorePropertyLooseInst %3 : object, globalObject : object, "x" : string
// CHECK-NEXT:  %13 = LoadStackInst %1
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function "get a"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst "a" : string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get 1"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 21 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "set 1"(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
