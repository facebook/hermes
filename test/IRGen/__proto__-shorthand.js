/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that __proto__ is not handled specially with shorthand
// syntax.

// __proto__ *should* be set as an "own" property.
function protoShorthand(func) {
  var __proto__ = 42;
  return {__proto__, a: 2, b: 3};
}

// __proto__ with shorthand syntax is a regular own property that allows
// duplication.
function protoShorthandDup(func) {
  var __proto__ = 42;
  return {__proto__, __proto__};
}

// __proto__: AssignmentExpression syntax mixed with shorthand syntax.
function protoShorthandMix1(func) {
  var __proto__ = 42;
  return {__proto__, __proto__: {}};
}

function protoShorthandMix2(func) {
  var __proto__ = 42;
  return {__proto__: {}, __proto__};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "protoShorthand": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoShorthandDup": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoShorthandMix1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoShorthandMix2": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %protoShorthand(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "protoShorthand": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %protoShorthandDup(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "protoShorthandDup": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %protoShorthandMix1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "protoShorthandMix1": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %protoShorthandMix2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "protoShorthandMix2": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [func: any, __proto__: any]

// CHECK:function protoShorthand(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.func]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.__proto__]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 42: number, [%VS1.__proto__]: any
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS1.__proto__]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst %7: any, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %6: object, "a": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst 3: number, %6: object, "b": string, true: boolean
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [func: any, __proto__: any]

// CHECK:function protoShorthandDup(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.func]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.__proto__]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 42: number, [%VS2.__proto__]: any
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS2.__proto__]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS2.__proto__]: any
// CHECK-NEXT:        DefineOwnPropertyInst %9: any, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [func: any, __proto__: any]

// CHECK:function protoShorthandMix1(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.func]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.__proto__]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 42: number, [%VS3.__proto__]: any
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS3.__proto__]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst %7: any, %6: object, "__proto__": string, true: boolean
// CHECK-NEXT:  %9 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: object, %9: object
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [func: any, __proto__: any]

// CHECK:function protoShorthandMix2(func: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.func]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.__proto__]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 42: number, [%VS4.__proto__]: any
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) %6: object
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS4.__proto__]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst %8: any, %7: object, "__proto__": string, true: boolean
// CHECK-NEXT:        ReturnInst %7: object
// CHECK-NEXT:function_end
