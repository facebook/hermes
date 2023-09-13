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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): undefined
// CHECK-NEXT:  %1 = CallInst (:undefined) %0: object, %""(): undefined, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): number|bigint
// CHECK-NEXT:  %1 = CallInst (:number|bigint) %0: object, %foo(): number|bigint, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = UnaryTypeofInst (:string) %1: number|bigint
// CHECK-NEXT:  %4 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: string, %1: number|bigint
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number|bigint [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %o(): undefined
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %" 1#"(): bigint
// CHECK-NEXT:       StorePropertyLooseInst %1: object, %0: object, "valueOf": string
// CHECK-NEXT:  %3 = UnaryMinusInst (:number|bigint) %0: object
// CHECK-NEXT:       ReturnInst %3: number|bigint
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
