/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ir -O -fno-inline -non-strict %s | %FileCheckOrRegen --match-full-lines %s
//
// Ensure that Hermes-specific optimizations (parameter type inference) are performed in non-strict
// mode. We need to disable inlining because it inlines foo() completely and we can't see the
// inference.

function main()  {
  function foo(p1) {
    return "value" + p1;
  }
  foo(2)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %main#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function main#0#1()#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{main#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#1#2()#3 : string, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 2 : number
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#1#2(p1 : number)#3 : string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#1#2()#3}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', "value" : string, 2 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : string
// CHECK-NEXT:function_end
