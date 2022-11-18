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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = AllocStackInst $nextIndex
// CHECK-NEXT:  %2 = StoreStackInst 0 : number, %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %3, 1 : number
// CHECK-NEXT:  %5 = StoreStackInst %4, %1
// CHECK-NEXT:  %6 = LoadStackInst %1
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %6, 1 : number
// CHECK-NEXT:  %8 = StoreStackInst %7, %1
// CHECK-NEXT:  %9 = LoadFrameInst [x]
// CHECK-NEXT:  %10 = AllocArrayInst 4 : number, 1 : number, 2 : number
// CHECK-NEXT:  %11 = LoadStackInst %1
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %10 : object, %9, %11
// CHECK-NEXT:  %13 = StoreStackInst %12, %1
// CHECK-NEXT:  %14 = LoadStackInst %1
// CHECK-NEXT:  %15 = StoreOwnPropertyInst 3 : number, %10 : object, %14, true : boolean
// CHECK-NEXT:  %16 = LoadStackInst %1
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 1 : number
// CHECK-NEXT:  %18 = StoreStackInst %17, %1
// CHECK-NEXT:  %19 = LoadStackInst %1
// CHECK-NEXT:  %20 = StoreOwnPropertyInst 4 : number, %10 : object, %19, true : boolean
// CHECK-NEXT:  %21 = LoadStackInst %1
// CHECK-NEXT:  %22 = BinaryOperatorInst '+', %21, 1 : number
// CHECK-NEXT:  %23 = StoreStackInst %22, %1
// CHECK-NEXT:  %24 = ReturnInst %10 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %25 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// OPT:function global() : undefined
// OPT-NEXT:frame = [], globals = [foo]
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateFunctionInst %foo() : object
// OPT-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// OPT-NEXT:  %2 = ReturnInst undefined : undefined
// OPT-NEXT:function_end

// OPT:function foo(x) : object
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = AllocArrayInst 4 : number, 1 : number, 2 : number
// OPT-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %0 : object, %x, 2 : number
// OPT-NEXT:  %2 = StoreOwnPropertyInst 3 : number, %0 : object, %1, true : boolean
// OPT-NEXT:  %3 = BinaryOperatorInst '+', %1, 1 : number
// OPT-NEXT:  %4 = StoreOwnPropertyInst 4 : number, %0 : object, %3 : string|number, true : boolean
// OPT-NEXT:  %5 = ReturnInst %0 : object
// OPT-NEXT:function_end
