/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  method(): number {
    return 1;
  }
}

print(new C().method());

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:       StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, C: any, ?C.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %3: object, [C]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %method(): functionCode
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) "method": string, %5: object
// CHECK-NEXT:       StoreFrameInst %6: object, [?C.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %3: object, "prototype": string
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [C]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:object) %10: any, type(object)
// CHECK-NEXT:  %12 = LoadFrameInst (:object) [?C.prototype]: object
// CHECK-NEXT:  %13 = UnionNarrowTrustedInst (:object) %12: object
// CHECK-NEXT:  %14 = AllocObjectInst (:object) 0: number, %13: object
// CHECK-NEXT:  %15 = LoadParentInst (:object) %14: object
// CHECK-NEXT:  %16 = PrLoadInst (:object) %15: object, 0: number, "method": string
// CHECK-NEXT:  %17 = CallInst [njsf] (:any) %16: object, empty: any, empty: any, undefined: undefined, %14: object
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:number) %17: any, type(number)
// CHECK-NEXT:  %19 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %18: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function method(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end
