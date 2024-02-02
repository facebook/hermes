/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  override(): number {
    return 1;
  }
  override2(x: number): number|string {
    return 1;
  }
}

class D extends C {
  constructor() {
    'use strict'
    super();
  }
  override(): number {
    return 2;
  }
  override2(x: number|string): number {
    return 2;
  }
}

function foo(c: C, d: D){
  c.override();
  d.override();
}

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
// CHECK-NEXT:frame = [exports: any, C: any, D: any, foo: any, ?C.prototype: object, ?D.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [D]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %4: object, [foo]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %6: object, [C]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %override(): functionCode
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %override2(): functionCode
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) "override": string, %8: object, "override2": string, %9: object
// CHECK-NEXT:        StoreFrameInst %10: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %10: object, %6: object, "prototype": string
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [C]: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:object) %13: any, type(object)
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %15: object, [D]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:object) [?C.prototype]: object
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %"override 1#"(): functionCode
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %"override2 1#"(): functionCode
// CHECK-NEXT:  %20 = AllocObjectLiteralInst (:object) "override": string, %18: object, "override2": string, %19: object
// CHECK-NEXT:        StoreParentInst %17: object, %20: object
// CHECK-NEXT:        StoreFrameInst %20: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %20: object, %15: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(c: object, d: object): any [typed]
// CHECK-NEXT:frame = [c: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %c: object
// CHECK-NEXT:       StoreFrameInst %0: object, [c]: any
// CHECK-NEXT:  %2 = LoadParamInst (:object) %d: object
// CHECK-NEXT:       StoreFrameInst %2: object, [d]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [c]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = LoadParentInst (:object) %5: object
// CHECK-NEXT:  %7 = PrLoadInst (:object) %6: object, 0: number, "override": string
// CHECK-NEXT:  %8 = CallInst [njsf] (:any) %7: object, empty: any, empty: any, undefined: undefined, %5: object
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:number) %8: any, type(number)
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [d]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:object) %10: any, type(object)
// CHECK-NEXT:  %12 = LoadParentInst (:object) %11: object
// CHECK-NEXT:  %13 = PrLoadInst (:object) %12: object, 0: number, "override": string
// CHECK-NEXT:  %14 = CallInst [njsf] (:any) %13: object, %"override 1#"(): functionCode, empty: any, undefined: undefined, %11: object
// CHECK-NEXT:  %15 = CheckedTypeCastInst (:number) %14: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function override(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function override2(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function D(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [C@""]: any
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:object) %1: any, type(object)
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, empty: any, %3: undefined|object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "override 1#"(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:function "override2 1#"(x: string|number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:string|number) %x: string|number
// CHECK-NEXT:       StoreFrameInst %0: string|number, [x]: any
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
