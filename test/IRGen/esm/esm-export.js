/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

export default function myFun() {
  return 1038;
}

export var x = 1;

export var y = 2, z = 4;

var longVariableName = 3;
var a = 4;

export { a, longVariableName as b }

export * from 'foo.js';

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function cjs_module#0#1(exports, require, module)#2
// CHECK-NEXT:frame = [exports#2, require#2, module#2, x#2, y#2, z#2, longVariableName#2, a#2, myFun#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{cjs_module#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %exports, [exports#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %require, [require#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %module, [module#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [x#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [y#2], %0
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [z#2], %0
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [longVariableName#2], %0
// CHECK-NEXT:  %8 = StoreFrameInst undefined : undefined, [a#2], %0
// CHECK-NEXT:  %9 = CreateFunctionInst %myFun#1#2()#3, %0
// CHECK-NEXT:  %10 = StoreFrameInst %9 : closure, [myFun#2], %0
// CHECK-NEXT:  %11 = LoadFrameInst [myFun#2], %0
// CHECK-NEXT:  %12 = StorePropertyInst %11, %exports, "myFun" : string
// CHECK-NEXT:  %13 = StoreFrameInst 1 : number, [x#2], %0
// CHECK-NEXT:  %14 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %15 = StorePropertyInst %14, %exports, "x" : string
// CHECK-NEXT:  %16 = StoreFrameInst 2 : number, [y#2], %0
// CHECK-NEXT:  %17 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %18 = StorePropertyInst %17, %exports, "y" : string
// CHECK-NEXT:  %19 = StoreFrameInst 4 : number, [z#2], %0
// CHECK-NEXT:  %20 = LoadFrameInst [z#2], %0
// CHECK-NEXT:  %21 = StorePropertyInst %20, %exports, "z" : string
// CHECK-NEXT:  %22 = StoreFrameInst 3 : number, [longVariableName#2], %0
// CHECK-NEXT:  %23 = StoreFrameInst 4 : number, [a#2], %0
// CHECK-NEXT:  %24 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %25 = StorePropertyInst %24, %exports, "a" : string
// CHECK-NEXT:  %26 = LoadFrameInst [longVariableName#2], %0
// CHECK-NEXT:  %27 = StorePropertyInst %26, %exports, "b" : string
// CHECK-NEXT:  %28 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %29 = CallBuiltinInst [HermesBuiltin.exportAll] : number, undefined : undefined, %exports, %28
// CHECK-NEXT:  %30 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function myFun#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{myFun#1#2()#3}
// CHECK-NEXT:  %1 = ReturnInst 1038 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
