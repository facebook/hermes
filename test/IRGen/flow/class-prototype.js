/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -Werror -O0 -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  method(): number {
    return 1;
  }
}

class D extends C {
  own(): number {
    return 2;
  }
}

// `.prototype` is a named property load on the constructor, and the method is
// then read directly from the resulting home object: no TypedLoadParentInst,
// because the home object is not the parent of itself. The IDZ check comes
// from the field type; home object slots are in fact always initialized.
C.prototype.method.call(new C());
D.prototype.method.call(new D());
D.prototype.own.call(new D());

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, C: any, D: any, ?C.prototype: object, ?D.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.D]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.C]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %method(): functionCode
// CHECK-NEXT:  %9 = AllocTypedNonEnumObjectInst (:object) null: null, "method": string, %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %6: object, "prototype": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %VS1: any, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.D]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %17 = PrLoadInst (:object) %16: object, 0: number, "method": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %1: environment, %VS1: any, %own(): functionCode
// CHECK-NEXT:  %19 = AllocTypedNonEnumObjectInst (:object) %16: object, "method": string, %17: object, "own": string, %18: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: object, [%VS1.?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %19: object, %14: object, "prototype": string
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %23 = CheckedTypeCastInst (:object) %22: any, type(object)
// CHECK-NEXT:  %24 = LoadPropertyInst (:object) %23: object, "prototype": string
// CHECK-NEXT:  %25 = PrLoadInst (:uninit|object) %24: object, 0: number, "method": string
// CHECK-NEXT:  %26 = ThrowIfInst (:object) %25: uninit|object, type(uninit)
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %28 = CheckedTypeCastInst (:object) %27: any, type(object)
// CHECK-NEXT:  %29 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %30 = UnionNarrowTrustedInst (:object) %29: object
// CHECK-NEXT:  %31 = AllocTypedObjectInst (:object) %30: object
// CHECK-NEXT:  %32 = CallInst (:any) %26: object, empty: any, false: boolean, empty: any, undefined: undefined, %31: object
// CHECK-NEXT:  %33 = CheckedTypeCastInst (:number) %32: any, type(number)
// CHECK-NEXT:  %34 = LoadFrameInst (:any) %1: environment, [%VS1.D]: any
// CHECK-NEXT:  %35 = CheckedTypeCastInst (:object) %34: any, type(object)
// CHECK-NEXT:  %36 = LoadPropertyInst (:object) %35: object, "prototype": string
// CHECK-NEXT:  %37 = PrLoadInst (:uninit|object) %36: object, 0: number, "method": string
// CHECK-NEXT:  %38 = ThrowIfInst (:object) %37: uninit|object, type(uninit)
// CHECK-NEXT:  %39 = LoadFrameInst (:any) %1: environment, [%VS1.D]: any
// CHECK-NEXT:  %40 = CheckedTypeCastInst (:object) %39: any, type(object)
// CHECK-NEXT:  %41 = LoadFrameInst (:object) %1: environment, [%VS1.?D.prototype]: object
// CHECK-NEXT:  %42 = UnionNarrowTrustedInst (:object) %41: object
// CHECK-NEXT:  %43 = AllocTypedObjectInst (:object) %42: object
// CHECK-NEXT:  %44 = CallInst (:any) %40: object, %D(): functionCode, true: boolean, empty: any, %40: object, %43: object
// CHECK-NEXT:  %45 = CallInst (:any) %38: object, empty: any, false: boolean, empty: any, undefined: undefined, %43: object
// CHECK-NEXT:  %46 = CheckedTypeCastInst (:number) %45: any, type(number)
// CHECK-NEXT:  %47 = LoadFrameInst (:any) %1: environment, [%VS1.D]: any
// CHECK-NEXT:  %48 = CheckedTypeCastInst (:object) %47: any, type(object)
// CHECK-NEXT:  %49 = LoadPropertyInst (:object) %48: object, "prototype": string
// CHECK-NEXT:  %50 = PrLoadInst (:uninit|object) %49: object, 1: number, "own": string
// CHECK-NEXT:  %51 = ThrowIfInst (:object) %50: uninit|object, type(uninit)
// CHECK-NEXT:  %52 = LoadFrameInst (:any) %1: environment, [%VS1.D]: any
// CHECK-NEXT:  %53 = CheckedTypeCastInst (:object) %52: any, type(object)
// CHECK-NEXT:  %54 = LoadFrameInst (:object) %1: environment, [%VS1.?D.prototype]: object
// CHECK-NEXT:  %55 = UnionNarrowTrustedInst (:object) %54: object
// CHECK-NEXT:  %56 = AllocTypedObjectInst (:object) %55: object
// CHECK-NEXT:  %57 = CallInst (:any) %53: object, %D(): functionCode, true: boolean, empty: any, %53: object, %56: object
// CHECK-NEXT:  %58 = CallInst (:any) %51: object, empty: any, false: boolean, empty: any, undefined: undefined, %56: object
// CHECK-NEXT:  %59 = CheckedTypeCastInst (:number) %58: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function C(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function method(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function D(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function own(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
