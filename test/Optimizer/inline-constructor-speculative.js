/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test that construction setup instructions like CreateThis and
// GetConstructedObject can be simplified even when inlining is speculative.
function foo(sink) {
  function newMyCons(){
    return new MyCons();
  }
  sink(newMyCons);

  function newMyClass(){
    return new MyClass();
  }
  sink(newMyClass);

  // Create an ordinary function, which should receive an object as 'this'.
  let MyCons = function () {
    "inline";
    sink(this);
  }

  // Ensure MyClass is a derived class to test the case where CreateThis
  // produces undefined.
  class MyClass extends MyCons {
    constructor() {
      "inline";
      super();
      sink(this);
    }
  }
};

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [sink: any, MyCons: undefined|object, MyClass: undefined|object, ?MyClass: object]

// CHECK:function foo(sink: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.sink]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.MyCons]: undefined|object
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.MyClass]: undefined|object
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %newMyCons(): functionCode
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %VS1: any, %newMyClass(): functionCode
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: object
// CHECK-NEXT:  %9 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %7: object
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %VS1: any, %MyCons(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.MyCons]: undefined|object
// CHECK-NEXT:  %12 = AllocStackInst (:object) $?anon_0_clsPrototype: any
// CHECK-NEXT:  %13 = CreateClassInst (:object) %1: environment, %VS1: any, %MyClass(): functionCode, %10: object, %12: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.?MyClass]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.MyClass]: undefined|object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function newMyCons(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|object) %0: environment, [%VS1.MyCons]: undefined|object
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: undefined|object, empty: any
// CHECK-NEXT:  %3 = TypeOfIsInst (:boolean) %1: undefined|object, typeOfIs(Function)
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %0: environment, [%VS1.sink]: any
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function newMyClass(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|object) %0: environment, [%VS1.MyClass]: undefined|object
// CHECK-NEXT:  %2 = CreateThisInst (:undefined|object) %1: undefined|object, empty: any
// CHECK-NEXT:  %3 = CallInst (:object) %1: undefined|object, %MyClass(): functionCode, false: boolean, %0: environment, %1: undefined|object, undefined: undefined
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function MyCons(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [%VS1.sink]: any
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:derived constructor MyClass(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %0: environment, [%VS1.?MyClass]: object
// CHECK-NEXT:  %3 = LoadParentNoTrapsInst (:null|object) %2: object
// CHECK-NEXT:  %4 = CreateThisInst (:undefined|object) %3: null|object, %1: object
// CHECK-NEXT:  %5 = CallInst (:any) %3: null|object, empty: any, false: boolean, empty: any, %1: object, %4: undefined|object
// CHECK-NEXT:  %6 = GetConstructedObjectInst (:object) %4: undefined|object, %5: any
// CHECK-NEXT:       ThrowIfThisInitializedInst empty: empty
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %0: environment, [%VS1.sink]: any
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: object
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end
