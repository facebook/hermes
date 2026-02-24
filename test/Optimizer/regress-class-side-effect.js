/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that the call to MyClass in makeClass is inlined.

function foo() {
  class MyClass {
    constructor(p1) {
      this.p1 = p1;
    }
  }
  function makeClass(p1){
    new MyClass(p1);
  }
  return makeClass;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [MyClass: object]

// CHECK:function foo(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %makeClass(): functionCode
// CHECK-NEXT:  %2 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %3 = CreateClassInst (:object) empty: any, empty: any, %MyClass(): functionCode, empty: any, %2: object
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: object, [%VS0.MyClass]: object
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function makeClass(p1: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS0.MyClass]: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) %3: any
// CHECK-NEXT:       StorePropertyStrictInst %1: any, %4: object, "p1": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor MyClass(p1: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) %1: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:       StorePropertyStrictInst %3: any, %2: object, "p1": string
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end
