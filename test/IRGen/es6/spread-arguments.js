/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -dump-ir -O %s | %FileCheck --match-full-lines -check-prefix=OPT %s

function foo(fn, x) {
  fn(...x);
  new fn(...x);
}
//CHECK-LABEL:function foo(fn, x)
//CHECK-NEXT:frame = [fn, x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst %fn, [fn]
//CHECK-NEXT:  %1 = StoreFrameInst %x, [x]
//CHECK-NEXT:  %2 = LoadFrameInst [fn]
//CHECK-NEXT:  %3 = AllocStackInst $nextIndex
//CHECK-NEXT:  %4 = StoreStackInst 0 : number, %3
//CHECK-NEXT:  %5 = LoadFrameInst [x]
//CHECK-NEXT:  %6 = AllocArrayInst 0 : number
//CHECK-NEXT:  %7 = LoadStackInst %3
//CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %6 : object, %5, %7
//CHECK-NEXT:  %9 = StoreStackInst %8, %3
//CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %2, %6 : object, undefined : undefined
//CHECK-NEXT:  %11 = LoadFrameInst [fn]
//CHECK-NEXT:  %12 = AllocStackInst $nextIndex
//CHECK-NEXT:  %13 = StoreStackInst 0 : number, %12
//CHECK-NEXT:  %14 = LoadFrameInst [x]
//CHECK-NEXT:  %15 = AllocArrayInst 0 : number
//CHECK-NEXT:  %16 = LoadStackInst %12
//CHECK-NEXT:  %17 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %15 : object, %14, %16
//CHECK-NEXT:  %18 = StoreStackInst %17, %12
//CHECK-NEXT:  %19 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %11, %15 : object
//CHECK-NEXT:  %20 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//OPT-LABEL:function foo(fn, x) : undefined
//OPT-NEXT:frame = []
//OPT-NEXT:%BB0:
//OPT-NEXT:  %0 = AllocArrayInst 0 : number
//OPT-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %0 : object, %x, 0 : number
//OPT-NEXT:  %2 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %fn, %0 : object, undefined : undefined
//OPT-NEXT:  %3 = AllocArrayInst 0 : number
//OPT-NEXT:  %4 = CallBuiltinInst [HermesBuiltin.arraySpread] : number, undefined : undefined, %3 : object, %x, 0 : number
//OPT-NEXT:  %5 = CallBuiltinInst [HermesBuiltin.apply] : number, undefined : undefined, %fn, %3 : object
//OPT-NEXT:  %6 = ReturnInst undefined : undefined
//OPT-NEXT:function_end
