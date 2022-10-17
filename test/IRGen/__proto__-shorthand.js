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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [protoShorthand, protoShorthandDup, protoShorthandMix1, protoShorthandMix2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %protoShorthand#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "protoShorthand" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %protoShorthandDup#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "protoShorthandDup" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %protoShorthandMix1#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "protoShorthandMix1" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %protoShorthandMix2#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "protoShorthandMix2" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function protoShorthand#0#1(func)#2
// CHECK-NEXT:frame = [func#2, __proto__#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoShorthand#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [__proto__#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst 42 : number, [__proto__#2], %0
// CHECK-NEXT:  %4 = LoadFrameInst [__proto__#2], %0
// CHECK-NEXT:  %5 = AllocObjectLiteralInst "__proto__" : string, %4, "a" : string, 2 : number, "b" : string, 3 : number
// CHECK-NEXT:  %6 = ReturnInst %5 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandDup#0#1(func)#3
// CHECK-NEXT:frame = [func#3, __proto__#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoShorthandDup#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [__proto__#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst 42 : number, [__proto__#3], %0
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %5 = LoadFrameInst [__proto__#3], %0
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst null : null, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = LoadFrameInst [__proto__#3], %0
// CHECK-NEXT:  %8 = StoreOwnPropertyInst %7, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %9 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandMix1#0#1(func)#4
// CHECK-NEXT:frame = [func#4, __proto__#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoShorthandMix1#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [__proto__#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst 42 : number, [__proto__#4], %0
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %5 = LoadFrameInst [__proto__#4], %0
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %8 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %4 : object, %7 : object
// CHECK-NEXT:  %9 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandMix2#0#1(func)#5
// CHECK-NEXT:frame = [func#5, __proto__#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoShorthandMix2#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [__proto__#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst 42 : number, [__proto__#5], %0
// CHECK-NEXT:  %4 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %5 = AllocObjectInst 1 : number, %4 : object
// CHECK-NEXT:  %6 = LoadFrameInst [__proto__#5], %0
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %6, %5 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %5 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
