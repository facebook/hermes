/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// This test shows that require calls with the same module index are CSE'd
// a dominating occurrence will be used for a subsequent occurrence.

function fakeRequire(n) {
  switch (n) {
  case 7:
    return {blah: 100, bz: 200};
  case 8:
    return {foo: 1000};
  default:
    return null;
  }
}

print($SHBuiltin.moduleFactory(
  0,
  function(glob, require) {
    // The second require(7) should not have a call -- it should
    // re-use the result of the first.
    return require(7).blah + require(8).foo + require(7).baz;
  })(undefined, fakeRequire));

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "fakeRequire": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %fakeRequire(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "fakeRequire": string
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %""(): functionCode
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "fakeRequire": string
// CHECK-NEXT:  %6 = CallInst (:string|number|bigint) %4: object, %""(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined, undefined: undefined, %5: any
// CHECK-NEXT:  %7 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: string|number|bigint
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function fakeRequire(n: any): null|object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       SwitchInst %0: any, %BB3, 7: number, %BB1, 8: number, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) empty: any, "blah": string, 100: number, "bz": string, 200: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any, "foo": string, 1000: number
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst null: null
// CHECK-NEXT:function_end

// CHECK:function ""(glob: any, require: any): string|number|bigint [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %1 = CallInst [metro-require] (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 7: number
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: any, "blah": string
// CHECK-NEXT:  %3 = CallInst [metro-require] (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 8: number
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "foo": string
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number|bigint) %2: any, %4: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %1: any, "baz": string
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number|bigint) %5: string|number|bigint, %6: any
// CHECK-NEXT:       ReturnInst %7: string|number|bigint
// CHECK-NEXT:function_end
