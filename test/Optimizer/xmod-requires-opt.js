/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// This test simulates the Metro requires tranform, and shows that
// requires calls in a module are compiled to a bytecode instruction.

// The important case is the call in the module factory function
// modFact1, which calls "require(0)".  This call should be
// annotated with the [metro-require] attribute.

function require(modIdx) {
  switch (modIdx) {
  case 0: {
    return $SHBuiltin.moduleFactory(
      0,
      function modFact0(global, require, module, exports) {
        function bar() {
          return 17;
        }
        exports.bar = bar;
        return exports;
      })(undefined, require, mod, exports);
  }
  case 1: {
    return $SHBuiltin.moduleFactory(
      1,
      function modFact1(global, require, module, exports) {
        // The require(0) should be annotated as a [metro-require].
        var x = require(0).bar();
        exports.x = x;
        return exports;
      })(undefined, require, mod, exports);
  }
  default:
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "require": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %require(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "require": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function require(modIdx: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %modIdx: any
// CHECK-NEXT:       SwitchInst %0: any, %BB1, 0: number, %BB2, 1: number, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) empty: any, empty: any, %modFact0(): functionCode
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "require": string
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "mod": string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "exports": string
// CHECK-NEXT:  %7 = CallInst (:any) %3: object, %modFact0(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined, undefined: undefined, %4: any, %5: any, %6: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) empty: any, empty: any, %modFact1(): functionCode
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) globalObject: object, "require": string
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "mod": string
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "exports": string
// CHECK-NEXT:  %13 = CallInst (:any) %9: object, %modFact1(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined, undefined: undefined, %10: any, %11: any, %12: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function modFact0(global: any, require: any, module: any, exports: any): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, %0: any, "bar": string
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function modFact1(global: any, require: any, module: any, exports: any): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %2 = CallInst [metro-require] (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "bar": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: any
// CHECK-NEXT:       StorePropertyLooseInst %4: any, %1: any, "x": string
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:function bar(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 17: number
// CHECK-NEXT:function_end
