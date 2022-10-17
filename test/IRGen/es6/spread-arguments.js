/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -dump-ir -O %s | %FileCheckOrRegen --match-full-lines -check-prefix=OPT %s

function foo(fn, x) {
  fn(...x);
  new fn(...x);
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

// CHECK:function foo#0#1(fn, x)#2
// CHECK-NEXT:frame = [fn#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %fn, [fn#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [fn#2], %0
// CHECK-NEXT:  %4 = AllocStackInst $nextIndex
// CHECK-NEXT:  %5 = StoreStackInst 0 : number, %4
// CHECK-NEXT:  %6 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %7 = AllocArrayInst 0 : number
// CHECK-NEXT:  %8 = LoadStackInst %4
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %7 : object, %6, %8
// CHECK-NEXT:  %10 = StoreStackInst %9, %4
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %3, %7 : object, undefined : undefined
// CHECK-NEXT:  %12 = LoadFrameInst [fn#2], %0
// CHECK-NEXT:  %13 = AllocStackInst $nextIndex
// CHECK-NEXT:  %14 = StoreStackInst 0 : number, %13
// CHECK-NEXT:  %15 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %16 = AllocArrayInst 0 : number
// CHECK-NEXT:  %17 = LoadStackInst %13
// CHECK-NEXT:  %18 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %16 : object, %15, %17
// CHECK-NEXT:  %19 = StoreStackInst %18, %13
// CHECK-NEXT:  %20 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %12, %16 : object
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// OPT:function global#0()#1 : undefined
// OPT-NEXT:frame = [], globals = [foo]
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// OPT-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2 : undefined, %0
// OPT-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// OPT-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-NEXT:function_end

// OPT:function foo#0#1(fn, x)#2 : undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// OPT-NEXT:  %1 = AllocArrayInst 0 : number
// OPT-NEXT:  %2 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %1 : object, %x, 0 : number
// OPT-NEXT:  %3 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %fn, %1 : object, undefined : undefined
// OPT-NEXT:  %4 = AllocArrayInst 0 : number
// OPT-NEXT:  %5 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %4 : object, %x, 0 : number
// OPT-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %fn, %4 : object
// OPT-NEXT:  %7 = ReturnInst undefined : undefined
// OPT-NEXT:function_end
