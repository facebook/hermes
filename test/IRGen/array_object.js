/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo(param) {
  var obj = {"1" : 2, "key" : param};

  var foo = [1,2,3,4];

  obj.field = foo;

  foo[5] = obj;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [foo]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(param)#2
// CHECK-NEXT:S{foo#0#1()#2} = [param#2, obj#2, foo#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %param, [param#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [obj#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [foo#2], %0
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2 : number, %4 : object, "1" : string, true : boolean
// CHECK-NEXT:  %6 = LoadFrameInst [param#2], %0
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %6, %4 : object, "key" : string, true : boolean
// CHECK-NEXT:  %8 = StoreFrameInst %4 : object, [obj#2], %0
// CHECK-NEXT:  %9 = AllocArrayInst 4 : number, 1 : number, 2 : number, 3 : number, 4 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9 : object, [foo#2], %0
// CHECK-NEXT:  %11 = LoadFrameInst [obj#2], %0
// CHECK-NEXT:  %12 = LoadFrameInst [foo#2], %0
// CHECK-NEXT:  %13 = StorePropertyInst %12, %11, "field" : string
// CHECK-NEXT:  %14 = LoadFrameInst [foo#2], %0
// CHECK-NEXT:  %15 = LoadFrameInst [obj#2], %0
// CHECK-NEXT:  %16 = StorePropertyInst %15, %14, 5 : number
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
