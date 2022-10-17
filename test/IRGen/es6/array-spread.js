/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines -check-prefix OPT %s

function foo(x) {
  return [1, 2, ...x, 3, 4];
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

// CHECK:function foo#0#1(x)#2
// CHECK-NEXT:frame = [x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = AllocStackInst $nextIndex
// CHECK-NEXT:  %3 = StoreStackInst 0 : number, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 1 : number
// CHECK-NEXT:  %6 = StoreStackInst %5, %2
// CHECK-NEXT:  %7 = LoadStackInst %2
// CHECK-NEXT:  %8 = BinaryOperatorInst '+', %7, 1 : number
// CHECK-NEXT:  %9 = StoreStackInst %8, %2
// CHECK-NEXT:  %10 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %11 = AllocArrayInst 4 : number, 1 : number, 2 : number
// CHECK-NEXT:  %12 = LoadStackInst %2
// CHECK-NEXT:  %13 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %11 : object, %10, %12
// CHECK-NEXT:  %14 = StoreStackInst %13, %2
// CHECK-NEXT:  %15 = LoadStackInst %2
// CHECK-NEXT:  %16 = StoreOwnPropertyInst 3 : number, %11 : object, %15, true : boolean
// CHECK-NEXT:  %17 = LoadStackInst %2
// CHECK-NEXT:  %18 = BinaryOperatorInst '+', %17, 1 : number
// CHECK-NEXT:  %19 = StoreStackInst %18, %2
// CHECK-NEXT:  %20 = LoadStackInst %2
// CHECK-NEXT:  %21 = StoreOwnPropertyInst 4 : number, %11 : object, %20, true : boolean
// CHECK-NEXT:  %22 = LoadStackInst %2
// CHECK-NEXT:  %23 = BinaryOperatorInst '+', %22, 1 : number
// CHECK-NEXT:  %24 = StoreStackInst %23, %2
// CHECK-NEXT:  %25 = ReturnInst %11 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %26 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// OPT:function global#0()#1 : undefined
// OPT-NEXT:frame = [], globals = [foo]
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2 : object, %0
// OPT-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// OPT-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-NEXT:function_end

// OPT:function foo#0#1(x)#2 : object
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// OPT-NEXT:  %1 = AllocArrayInst 4 : number, 1 : number, 2 : number
// OPT-NEXT:  %2 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %1 : object, %x, 2 : number
// OPT-NEXT:  %3 = StoreOwnPropertyInst 3 : number, %1 : object, %2, true : boolean
// OPT-NEXT:  %4 = BinaryOperatorInst '+', %2, 1 : number
// OPT-NEXT:  %5 = StoreOwnPropertyInst 4 : number, %1 : object, %4 : string|number, true : boolean
// OPT-NEXT:  %6 = ReturnInst %1 : object
// OPT-NEXT:function_end
