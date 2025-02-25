/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

(function (){
  function foo(){
    // x should be inlinable with a type check, because we know the only other
    // value stored to it is undefined.
    return x();
  }

  // foo escapes before x is initialized.
  globalThis.foo = foo;

  var x = () => 42;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [x: undefined|object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS0.x]: undefined|object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %3: any, "foo": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %x(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: object, [%VS0.x]: undefined|object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|object) %0: environment, [%VS0.x]: undefined|object
// CHECK-NEXT:  %2 = TypeOfIsInst (:boolean) %1: undefined|object, typeOfIs(Function)
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ThrowTypeErrorInst "Trying to call a non-function": string
// CHECK-NEXT:function_end

// CHECK:arrow x(): number [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
