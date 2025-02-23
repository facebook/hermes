/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -Xenable-tdz %s | %FileCheckOrRegen --match-full-lines %s

(function (){
  function foo(){
    // x should be inlinable after the TDZ check, because we know the only other
    // possible value is the arrow function below.
    return x();
  }

  // foo escapes before the TDZ var x is initialized.
  globalThis.foo = foo;

  let x = () => 42;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: empty|object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.x]: empty|object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %VS1: any, %foo(): functionCode
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:       StorePropertyLooseInst %3: object, %4: any, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %x(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.x]: empty|object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:empty|object) %0: environment, [%VS1.x]: empty|object
// CHECK-NEXT:  %2 = ThrowIfInst (:object) %1: empty|object, type(empty)
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:function_end

// CHECK:arrow x(): number [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
