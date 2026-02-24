/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// This test verifies that PrivateBrandCheckDedup eliminates duplicate
// PrivateBrandCheckInst instructions when the same object/brand combination
// is checked multiple times within dominated blocks.

function testPrivateInstanceMethods() {
  class A {
    #m() { return 1; }

    callThreeInst(o) {
      // After PrivateBrandCheckDedup, there should be only ONE
      // PrivateBrandCheckInst. The second check is redundant because if the
      // first check passes, the brand is proven to be present.
      var a = o.#m();
      var b = o.#m();
      var c = o.#m();
      return a + b + c;
    }

    callOnDifferentObjectsInst(o1, o2) {
      // Checks cannot be eliminated because they are on different objects.
      var a = o1.#m();
      var b = o2.#m();
      return a + b;
    }

    callInBranchesInst(o, cond) {
      // Checks cannot be eliminated because they are on different paths.
      if (cond) {
        return o.#m();
      } else {
        return o.#m();
      }
    }
  }
  return A;
}

function testPrivateStaticMethods() {
  class B {
    static #s() { return 2; }

    static callThreeStatic(o) {
      var a = o.#s();
      var b = o.#s();
      var c = o.#s();
      return a + b + c;
    }

    static callOnDifferentObjectsStatic(o1, o2) {
      var a = o1.#s();
      var b = o2.#s();
      return a + b;
    }

    static callInBranchesStatic(o, cond) {
      if (cond) {
        return o.#s();
      } else {
        return o.#s();
      }
    }
  }
  return B;
}

function testPrivateAccessors() {
  class C {
    get #x() { return 3; }
    set #x(v) {}

    callThreeAccessor(o) {
      var a = o.#x;
      var b = o.#x;
      var c = o.#x;
      o.#x = a;
      o.#x = b;
      o.#x = c;
      return a + b + c;
    }

    callOnDifferentObjectsAccessor(o1, o2) {
      var a = o1.#x;
      var b = o2.#x;
      o1.#x = a;
      o2.#x = a;
      return a + b;
    }

    callInBranchesAccessor(o, cond) {
      if (cond) {
        o.#x = 1;
        return o.#x;
      } else {
        o.#x = 1;
        return o.#x;
      }
    }
  }
  return C;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "testPrivateInstanceMethods": string
// CHECK-NEXT:       DeclareGlobalVarInst "testPrivateStaticMethods": string
// CHECK-NEXT:       DeclareGlobalVarInst "testPrivateAccessors": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) empty: any, empty: any, %testPrivateInstanceMethods(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "testPrivateInstanceMethods": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) empty: any, empty: any, %testPrivateStaticMethods(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "testPrivateStaticMethods": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) empty: any, empty: any, %testPrivateAccessors(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "testPrivateAccessors": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [?instance_brand_A: privateName]

// CHECK:function testPrivateInstanceMethods(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreatePrivateNameInst (:privateName) "A": string
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: privateName, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:  %3 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %4 = CreateClassInst (:object) %0: environment, %VS0: any, %A(): functionCode, empty: any, %3: object
// CHECK-NEXT:  %5 = LoadStackInst (:object) %3: object
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %VS0: any, %callThreeInst(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %6: object, %5: object, "callThreeInst": string, false: boolean
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS0: any, %callOnDifferentObjectsInst(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %8: object, %5: object, "callOnDifferentObjectsInst": string, false: boolean
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %VS0: any, %callInBranchesInst(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %10: object, %5: object, "callInBranchesInst": string, false: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [?static_brand_B: privateName]

// CHECK:function testPrivateStaticMethods(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = CreatePrivateNameInst (:privateName) "B": string
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: privateName, [%VS1.?static_brand_B]: privateName
// CHECK-NEXT:  %3 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %4 = CreateClassInst (:object) empty: any, empty: any, %B(): functionCode, empty: any, %3: object
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS1: any, %callThreeStatic(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %5: object, %4: object, "callThreeStatic": string, false: boolean
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS1: any, %callOnDifferentObjectsStatic(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %7: object, %4: object, "callOnDifferentObjectsStatic": string, false: boolean
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS1: any, %callInBranchesStatic(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %9: object, %4: object, "callInBranchesStatic": string, false: boolean
// CHECK-NEXT:        AddOwnPrivateFieldInst undefined: undefined, %4: object, %1: privateName
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [?instance_brand_C: privateName]

// CHECK:function testPrivateAccessors(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS2: any, empty: any
// CHECK-NEXT:  %1 = CreatePrivateNameInst (:privateName) "C": string
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: privateName, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:  %3 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %4 = CreateClassInst (:object) %0: environment, %VS2: any, %C(): functionCode, empty: any, %3: object
// CHECK-NEXT:  %5 = LoadStackInst (:object) %3: object
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %VS2: any, %callThreeAccessor(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %6: object, %5: object, "callThreeAccessor": string, false: boolean
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS2: any, %callOnDifferentObjectsAccessor(): functionCode
// CHECK-NEXT:       DefineOwnPropertyInst %8: object, %5: object, "callOnDifferentObjectsAccessor": string, false: boolean
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %VS2: any, %callInBranchesAccessor(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %10: object, %5: object, "callInBranchesAccessor": string, false: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:base constructor A(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, "prototype": string
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) %2: any
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:  %5 = BinaryPrivateInInst (:boolean) %4: privateName, %3: object
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       AddOwnPrivateFieldInst undefined: undefined, %3: object, %4: privateName
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ThrowTypeErrorInst "Cannot initialize private field twice.": string
// CHECK-NEXT:function_end

// CHECK:method callThreeInst(o: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadFrameInst (:privateName) %0: environment, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %2: privateName
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:function_end

// CHECK:method callOnDifferentObjectsInst(o1: any, o2: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o1: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o2: any
// CHECK-NEXT:  %3 = LoadFrameInst (:privateName) %0: environment, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %3: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %2: any, %3: privateName
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:method callInBranchesInst(o: any, cond: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %4: privateName
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:privateName) %0: environment, [%VS0.?instance_brand_A]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %7: privateName
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:base constructor B(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) %1: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:method callThreeStatic(o: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadFrameInst (:privateName) %0: environment, [%VS1.?static_brand_B]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %2: privateName
// CHECK-NEXT:       ReturnInst 6: number
// CHECK-NEXT:function_end

// CHECK:method callOnDifferentObjectsStatic(o1: any, o2: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o1: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o2: any
// CHECK-NEXT:  %3 = LoadFrameInst (:privateName) %0: environment, [%VS1.?static_brand_B]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %3: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %2: any, %3: privateName
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:method callInBranchesStatic(o: any, cond: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS1.?static_brand_B]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %4: privateName
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:privateName) %0: environment, [%VS1.?static_brand_B]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %7: privateName
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:base constructor C(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, "prototype": string
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) %2: any
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:  %5 = BinaryPrivateInInst (:boolean) %4: privateName, %3: object
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       AddOwnPrivateFieldInst undefined: undefined, %3: object, %4: privateName
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ThrowTypeErrorInst "Cannot initialize private field twice.": string
// CHECK-NEXT:function_end

// CHECK:method callThreeAccessor(o: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadFrameInst (:privateName) %0: environment, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %2: privateName
// CHECK-NEXT:       ReturnInst 9: number
// CHECK-NEXT:function_end

// CHECK:method callOnDifferentObjectsAccessor(o1: any, o2: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o1: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o2: any
// CHECK-NEXT:  %3 = LoadFrameInst (:privateName) %0: environment, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %3: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %2: any, %3: privateName
// CHECK-NEXT:       ReturnInst 6: number
// CHECK-NEXT:function_end

// CHECK:method callInBranchesAccessor(o: any, cond: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %o: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %cond: any
// CHECK-NEXT:       CondBranchInst %2: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst (:privateName) %0: environment, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %4: privateName
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = LoadFrameInst (:privateName) %0: environment, [%VS2.?instance_brand_C]: privateName
// CHECK-NEXT:       PrivateBrandCheckInst %1: any, %7: privateName
// CHECK-NEXT:       ReturnInst 3: number
// CHECK-NEXT:function_end
