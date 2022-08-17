/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ir %s | %FileCheck %s --match-full-lines

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

// CHECK-LABEL: function global()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:   %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:   %2 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:   %3 = StoreOwnPropertyInst 3 : number, %2 : object, "x" : string, true : boolean
// CHECK-NEXT:   %4 = CreateFunctionInst %""()
// CHECK-NEXT:   %5 = StoreGetterSetterInst %4 : closure, undefined : undefined, %2 : object, "y" : string, true : boolean
// CHECK-NEXT:   %6 = CreateFunctionInst %" 1#"()
// CHECK-NEXT:   %7 = StoreGetterSetterInst undefined : undefined, %6 : closure, %2 : object, "y" : string, true : boolean
// CHECK-NEXT:   %8 = CreateFunctionInst %" 2#"()
// CHECK-NEXT:   %9 = StoreOwnPropertyInst %8 : closure, %2 : object, "z" : string, true : boolean
// CHECK-NEXT:   %10 = StoreStackInst %2 : object, %0
// CHECK-NEXT:   %11 = LoadStackInst %0
// CHECK-NEXT:   %12 = ReturnInst %11
// CHECK-NEXT: function_end

// CHECK-LABEL: function ""()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = ReturnInst 42 : number
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// CHECK-LABEL: function " 1#"(val)
// CHECK-NEXT: frame = [val]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %val, [val]
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// CHECK-LABEL: function " 2#"()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = ReturnInst 100 : number
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
