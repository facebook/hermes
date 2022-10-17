/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo() {
  return "hi"
  return 2.312
  return 12
  return 0x12
  return true
  return undefined
  return null
}

foo()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = StoreStackInst %6, %3
// CHECK-NEXT:  %8 = LoadStackInst %3
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst "hi" : string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst 2.312 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst 12 : number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %4 = ReturnInst 18 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %5 = ReturnInst true : boolean
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %7 = ReturnInst null : null
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
