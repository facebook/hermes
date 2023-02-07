/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function sink0(a) { }
function sink1(a) { }

function test1(x,y) {
  return (1,2,3);
}

function test2(x,y) {
  return (sink0(x,y), sink1(x,y));
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink0" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "sink1" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test1" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test2" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %sink0()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "sink0" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %sink1()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "sink1" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %test1()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %test2()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function sink0(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink1(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = ReturnInst 3 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "sink0" : string
// CHECK-NEXT:  %5 = LoadFrameInst [x]
// CHECK-NEXT:  %6 = LoadFrameInst [y]
// CHECK-NEXT:  %7 = CallInst %4, empty, empty, undefined : undefined, %5, %6
// CHECK-NEXT:  %8 = LoadPropertyInst globalObject : object, "sink1" : string
// CHECK-NEXT:  %9 = LoadFrameInst [x]
// CHECK-NEXT:  %10 = LoadFrameInst [y]
// CHECK-NEXT:  %11 = CallInst %8, empty, empty, undefined : undefined, %9, %10
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
