/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(a, b, c) {
    return {a, ...b, c};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b, c)#2
// CHECK-NEXT:frame = [a#2, b#2, c#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#2], %0
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %5 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = LoadFrameInst [b#2], %0
// CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %4 : object, %7
// CHECK-NEXT:  %9 = LoadFrameInst [c#2], %0
// CHECK-NEXT:  %10 = StoreOwnPropertyInst %9, %4 : object, "c" : string, true : boolean
// CHECK-NEXT:  %11 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
