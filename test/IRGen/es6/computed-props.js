/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreOwnPropertyInst 3: number, %2: object, "x": string, true: boolean
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:       StoreGetterSetterInst %4: object, undefined: undefined, %2: object, "y": string, true: boolean
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %" 1#"(): any
// CHECK-NEXT:       StoreGetterSetterInst undefined: undefined, %6: object, %2: object, "y": string, true: boolean
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %" 2#"(): any
// CHECK-NEXT:       StoreOwnPropertyInst %8: object, %2: object, "z": string, true: boolean
// CHECK-NEXT:        StoreStackInst %2: object, %0: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %0: any
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function " 1#"(val: any): any
// CHECK-NEXT:frame = [val: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %val: any
// CHECK-NEXT:       StoreFrameInst %0: any, [val]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function " 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 100: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
