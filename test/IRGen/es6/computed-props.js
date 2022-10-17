/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

({
  ['x']: 3,
  get ['y']() {
    return 42;
  },
  set ['y'](val) {},
  ['z']: function() {
    return 100;
  },
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %4 = StoreOwnPropertyInst 3 : number, %3 : object, "x" : string, true : boolean
// CHECK-NEXT:  %5 = CreateFunctionInst %""#0#1()#2, %0
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5 : closure, undefined : undefined, %3 : object, "y" : string, true : boolean
// CHECK-NEXT:  %7 = CreateFunctionInst %" 1#"#0#1()#3, %0
// CHECK-NEXT:  %8 = StoreGetterSetterInst undefined : undefined, %7 : closure, %3 : object, "y" : string, true : boolean
// CHECK-NEXT:  %9 = CreateFunctionInst %" 2#"#0#1()#4, %0
// CHECK-NEXT:  %10 = StoreOwnPropertyInst %9 : closure, %3 : object, "z" : string, true : boolean
// CHECK-NEXT:  %11 = StoreStackInst %3 : object, %1
// CHECK-NEXT:  %12 = LoadStackInst %1
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function ""#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst 42 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 1#"#0#1(val)#3
// CHECK-NEXT:frame = [val#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{" 1#"#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %val, [val#3], %0
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 2#"#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{" 2#"#0#1()#4}
// CHECK-NEXT:  %1 = ReturnInst 100 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
