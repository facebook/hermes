/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s -fno-inline | %FileCheckOrRegen %s --match-full-lines

// Test that type inference can handle object->bigint conversion.
// The type of `v` needs to account for bigint after the unary operation.

(function() {
  function foo() {
    function o() {}
    o.valueOf = () => 10n;
    return -o;
  }

  var v = foo();
  // Check that the type of 'v' is correct.
  print(typeof v, v);
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %""(): functionCode, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:  %2 = CallInst (:number|bigint) %1: object, %foo(): functionCode, %0: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = UnaryTypeofInst (:string) %2: number|bigint
// CHECK-NEXT:  %5 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: string, %2: number|bigint
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number|bigint [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %o(): functionCode
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %" 1#"(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %1: object, "valueOf": string
// CHECK-NEXT:  %4 = UnaryMinusInst (:number|bigint) %1: object
// CHECK-NEXT:       ReturnInst %4: number|bigint
// CHECK-NEXT:function_end

// CHECK:function o(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 10n: bigint
// CHECK-NEXT:function_end
