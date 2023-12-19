/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -fno-inline -O0 %s | %FileCheckOrRegen --match-full-lines %s

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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:       StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, main: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %main(): any
// CHECK-NEXT:       StoreFrameInst %2: object, [main]: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): any
// CHECK-NEXT:frame = [foo: any, C: any, ?C.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StoreFrameInst %1: object, [foo]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:       StoreFrameInst %3: object, [C]: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %5: object, [?C.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %5: object, %3: object, "prototype": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [C@main]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:object) [?C.prototype@main]: object
// CHECK-NEXT:  %4 = UnionNarrowTrustedInst (:object) %3: object
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) "x": string, 0: number
// CHECK-NEXT:       StoreParentInst %4: object, %5: object
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, %C(): any, empty: any, %2: any, %5: object, %7: any
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:function C(x: any): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: any, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any
// CHECK-NEXT:       PrStoreInst %4: number, %0: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
