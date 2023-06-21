/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -fno-inline -O0 %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

(function main() {

function foo(x: number): C {
  // Ensure the target operand of new C() is populated at IRGen.
  return new C(x);
}

class C {
  x: number;
  constructor(x) {
    this.x = x;
  }
}

return foo;

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = [main: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = StoreStackInst "use strict": string, %0: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %main(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [main]: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = StoreStackInst %5: any, %0: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %8 = ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function main(): any [typed]
// CHECK-NEXT:frame = [foo: any, C: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [foo]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [C]: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %6 = StorePropertyStrictInst %5: object, %3: object, "prototype": string
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %8 = ReturnInst %7: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [C@main]: any
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "prototype": string
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) "x": string, 0: number
// CHECK-NEXT:  %5 = StoreParentInst %3: any, %4: object
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = CallInst (:any) %2: any, %C(): any, empty: any, %2: any, %4: object, %6: any
// CHECK-NEXT:  %8 = ReturnInst %4: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(x: any): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %2 = StoreFrameInst %1: any, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %4 = PrStoreInst %3: any, %0: any, 0: number, "x": string, true: boolean
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
