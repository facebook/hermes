/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// These are simple tests, to show that the module-related $SHBuiltin functions
// (moduleFactory, export, import) are "compiled away" when cross-module
// optimizations are not enabled.

// moduleFactory reduces to its second argument.  That's inlined.
// So the constant 7 is stored to x0.
var x0 = $SHBuiltin.moduleFactory(0, function () { return 7; })();

// export generates no code.
function foo() {
  var x = 7;
  $SHBuiltin.export("x", x);
  return x;
}

// import reduces to third argument.
var x1 = $SHBuiltin.import(100, "x", x0);

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "x0": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "x1": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:       StorePropertyLooseInst 7: number, globalObject: object, "x0": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "x0": string
// CHECK-NEXT:       StorePropertyLooseInst %7: any, globalObject: object, "x1": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 7: number
// CHECK-NEXT:function_end
