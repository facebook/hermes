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

// CHECK:function foo(param)
// CHECK-NEXT:frame = [obj, foo, param]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [obj]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [foo]
// CHECK-NEXT:  %2 = LoadParamInst %param
// CHECK-NEXT:  %3 = StoreFrameInst %2, [param]
// CHECK-NEXT:  %4 = LoadFrameInst [param]
// CHECK-NEXT:  %5 = AllocObjectLiteralInst "1" : string, 2 : number, "key" : string, %4
// CHECK-NEXT:  %6 = StoreFrameInst %5 : object, [obj]
// CHECK-NEXT:  %7 = AllocArrayInst 4 : number, 1 : number, 2 : number, 3 : number, 4 : number
// CHECK-NEXT:  %8 = StoreFrameInst %7 : object, [foo]
// CHECK-NEXT:  %9 = LoadFrameInst [obj]
// CHECK-NEXT:  %10 = LoadFrameInst [foo]
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10, %9, "field" : string
// CHECK-NEXT:  %12 = LoadFrameInst [foo]
// CHECK-NEXT:  %13 = LoadFrameInst [obj]
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13, %12, 5 : number
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
