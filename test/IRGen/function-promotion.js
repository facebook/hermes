/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
  let x;
  var y;
  {
    function x() {}
    function y() {}
    function z() {}
  }
  // x is not promoted.
  // y is promoted.
  // z is promoted.
  print(x, y, z);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo()
// CHECK-NEXT:frame = [x, y, z, x#1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %3 = CreateFunctionInst %x()
// CHECK-NEXT:  %4 = StoreFrameInst %3 : closure, [x#1]
// CHECK-NEXT:  %5 = CreateFunctionInst %y()
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [y]
// CHECK-NEXT:  %7 = CreateFunctionInst %z()
// CHECK-NEXT:  %8 = StoreFrameInst %7 : closure, [z]
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = LoadFrameInst [y]
// CHECK-NEXT:  %12 = LoadFrameInst [z]
// CHECK-NEXT:  %13 = CallInst %9, undefined : undefined, %10, %11, %12
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function x()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function y()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function z()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
