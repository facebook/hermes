/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=C,maker,method -O0 -Werror -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

class A {}
class B {}

class C {
  p1: number;
  p2: number | A;
  p3: void | A;
  p4: A;
  p5: A | B;

  constructor() {
    this.p1 = 1;
    this.p2 = 2;
    this.p3 = undefined;
  }

  maker(): C {
    return new C();
  }

  method() {
    // These should not have ThrowIf
    print(this.p1)
    print(this.p2)
    print(this.p3)
    // These should have ThrowIf
    print(this.p4)
    print(this.p5)
  }
}

return C;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [exports: any, A: any, B: any, C: any, ?A.prototype: object, ?B.prototype: object, ?C.prototype: object]

// CHECK:scope %VS1 []

// CHECK:base constructor C(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS1: any, %1: environment
// CHECK-NEXT:       PrStoreInst 1: number, %0: object, 0: number, "p1": string, true: boolean
// CHECK-NEXT:       PrStoreInst 2: number, %0: object, 1: number, "p2": string, false: boolean
// CHECK-NEXT:       PrStoreInst undefined: undefined, %0: object, 2: number, "p3": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function maker(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS0.C]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:object) %2: any, type(object)
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %0: environment, [%VS0.?C.prototype]: object
// CHECK-NEXT:  %5 = UnionNarrowTrustedInst (:object) %4: object
// CHECK-NEXT:  %6 = AllocTypedObjectInst (:object) %5: object, "p1": string, 0: number, "p2": string, 0: number, "p3": string, undefined: undefined, "p4": string, uninit: uninit, "p5": string, uninit: uninit
// CHECK-NEXT:  %7 = CallInst (:any) %3: object, %C(): functionCode, true: boolean, empty: any, %3: object, %6: object
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function method(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %5 = PrLoadInst (:number) %4: object, 0: number, "p1": string
// CHECK-NEXT:  %6 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: number
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %9 = PrLoadInst (:number|object) %8: object, 1: number, "p2": string
// CHECK-NEXT:  %10 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %9: number|object
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %13 = PrLoadInst (:undefined|object) %12: object, 2: number, "p3": string
// CHECK-NEXT:  %14 = CallInst (:any) %11: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %13: undefined|object
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %17 = PrLoadInst (:uninit|object) %16: object, 3: number, "p4": string
// CHECK-NEXT:  %18 = ThrowIfInst (:object) %17: uninit|object, type(uninit)
// CHECK-NEXT:  %19 = CallInst (:any) %15: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %18: object
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %21 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %22 = PrLoadInst (:uninit|object) %21: object, 4: number, "p5": string
// CHECK-NEXT:  %23 = ThrowIfInst (:object) %22: uninit|object, type(uninit)
// CHECK-NEXT:  %24 = CallInst (:any) %20: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %23: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
