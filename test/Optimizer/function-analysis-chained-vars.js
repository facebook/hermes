/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Function is threaded through multiple variables.
// Note that the call and env in 'bar' is correctly resolved.
function main() {
  var i = 0;
  var x = () => ++i;
  function foo() {
    var y = x;
    function bar() {
      var z = y;
      z();
    }
    return bar;
  }
  foo();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [i: number, x: object]

// CHECK:function main(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, 0: number, [%VS0.i]: number
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %x(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: object, [%VS0.x]: object
// CHECK-NEXT:  %5 = CallInst (:object) %1: object, %foo(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [y: object]

// CHECK:function foo(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS1: any, %bar(): functionCode
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %0: environment, [%VS0.x]: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS1.y]: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:arrow x(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS0.i]: number
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: number, [%VS0.i]: number
// CHECK-NEXT:       ReturnInst %2: number
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS1.y]: object
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS0: any, %VS1: any, %0: environment
// CHECK-NEXT:  %3 = CallInst (:number) %1: object, %x(): functionCode, true: boolean, %2: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
