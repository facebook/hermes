/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

main()

function main() {

  function foo(x) { return capture_me; }

  var capture_me;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %main#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "main" : string
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = StoreStackInst %6, %3
// CHECK-NEXT:  %8 = LoadStackInst %3
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function main#0#1()#2
// CHECK-NEXT:frame = [capture_me#2, foo#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{main#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [capture_me#2], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %foo#1#2()#3, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [foo#2], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#1#2(x)#3
// CHECK-NEXT:frame = [x#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#1#2()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [capture_me#2@main], %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
