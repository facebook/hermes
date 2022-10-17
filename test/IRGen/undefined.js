/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo() {
  var undefined = 5;
  return undefined;
}

var undefined = 5;
foo();
undefined;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = StorePropertyInst 5 : number, globalObject : object, "undefined" : string
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined
// CHECK-NEXT:  %8 = StoreStackInst %7, %3
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %10 = LoadStackInst %3
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:frame = [undefined#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [undefined#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst 5 : number, [undefined#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [undefined#2], %0
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
