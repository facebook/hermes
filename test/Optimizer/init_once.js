/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O | %FileCheckOrRegen --match-full-lines %s

function main(p) {
  var k = p;
  var p = print;
  function bar() {
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
    p(k)
  }

  return bar;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %main#0#1()#2 : closure, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function main#0#1(p)#2 : closure
// CHECK-NEXT:frame = [p#2, k#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{main#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %p, [p#2], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %bar#1#2()#3 : undefined, %0
// CHECK-NEXT:  %3 = StoreFrameInst %p, [k#2], %0
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = StoreFrameInst %4, [p#2], %0
// CHECK-NEXT:  %6 = ReturnInst %2 : closure
// CHECK-NEXT:function_end

// CHECK:function bar#1#2()#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#1#2()#3}
// CHECK-NEXT:  %1 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %2 = LoadFrameInst [k#2@main], %0
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined, %2
// CHECK-NEXT:  %6 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %7 = CallInst %6, undefined : undefined, %2
// CHECK-NEXT:  %8 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %9 = CallInst %8, undefined : undefined, %2
// CHECK-NEXT:  %10 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %11 = CallInst %10, undefined : undefined, %2
// CHECK-NEXT:  %12 = LoadFrameInst [p#2@main], %0
// CHECK-NEXT:  %13 = CallInst %12, undefined : undefined, %2
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
