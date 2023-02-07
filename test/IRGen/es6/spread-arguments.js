/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -dump-ir -O %s | %FileCheckOrRegen --match-full-lines -check-prefix=OPT %s

function foo(fn, x) {
  fn(...x);
  new fn(...x);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(fn, x)
// CHECK-NEXT:frame = [fn, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %fn
// CHECK-NEXT:  %1 = StoreFrameInst %0, [fn]
// CHECK-NEXT:  %2 = LoadParamInst %x
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [fn]
// CHECK-NEXT:  %5 = AllocStackInst $nextIndex
// CHECK-NEXT:  %6 = StoreStackInst 0 : number, %5
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = AllocArrayInst 0 : number
// CHECK-NEXT:  %9 = LoadStackInst %5
// CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %8 : object, %7, %9
// CHECK-NEXT:  %11 = StoreStackInst %10, %5
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %4, %8 : object, undefined : undefined
// CHECK-NEXT:  %13 = LoadFrameInst [fn]
// CHECK-NEXT:  %14 = AllocStackInst $nextIndex
// CHECK-NEXT:  %15 = StoreStackInst 0 : number, %14
// CHECK-NEXT:  %16 = LoadFrameInst [x]
// CHECK-NEXT:  %17 = AllocArrayInst 0 : number
// CHECK-NEXT:  %18 = LoadStackInst %14
// CHECK-NEXT:  %19 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %17 : object, %16, %18
// CHECK-NEXT:  %20 = StoreStackInst %19, %14
// CHECK-NEXT:  %21 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %13, %17 : object
// CHECK-NEXT:  %22 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// OPT:function global() : undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// OPT-NEXT:  %1 = CreateFunctionInst %foo() : undefined
// OPT-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// OPT-NEXT:  %3 = ReturnInst undefined : undefined
// OPT-NEXT:function_end

// OPT:function foo(fn, x) : undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = LoadParamInst %fn
// OPT-NEXT:  %1 = LoadParamInst %x
// OPT-NEXT:  %2 = AllocArrayInst 0 : number
// OPT-NEXT:  %3 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %2 : object, %1, 0 : number
// OPT-NEXT:  %4 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %0, %2 : object, undefined : undefined
// OPT-NEXT:  %5 = AllocArrayInst 0 : number
// OPT-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %5 : object, %1, 0 : number
// OPT-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %0, %5 : object
// OPT-NEXT:  %8 = ReturnInst undefined : undefined
// OPT-NEXT:function_end
