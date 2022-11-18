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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [protoShorthand, protoShorthandDup, protoShorthandMix1, protoShorthandMix2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %protoShorthand()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "protoShorthand" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %protoShorthandDup()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "protoShorthandDup" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %protoShorthandMix1()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "protoShorthandMix1" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %protoShorthandMix2()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "protoShorthandMix2" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function protoShorthand(func)
// CHECK-NEXT:frame = [__proto__, func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [__proto__]
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %2 = StoreFrameInst 42 : number, [__proto__]
// CHECK-NEXT:  %3 = LoadFrameInst [__proto__]
// CHECK-NEXT:  %4 = AllocObjectLiteralInst "__proto__" : string, %3, "a" : string, 2 : number, "b" : string, 3 : number
// CHECK-NEXT:  %5 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandDup(func)
// CHECK-NEXT:frame = [__proto__, func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [__proto__]
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %2 = StoreFrameInst 42 : number, [__proto__]
// CHECK-NEXT:  %3 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %4 = LoadFrameInst [__proto__]
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null : null, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = LoadFrameInst [__proto__]
// CHECK-NEXT:  %7 = StoreOwnPropertyInst %6, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandMix1(func)
// CHECK-NEXT:frame = [__proto__, func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [__proto__]
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %2 = StoreFrameInst 42 : number, [__proto__]
// CHECK-NEXT:  %3 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %4 = LoadFrameInst [__proto__]
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4, %3 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %6 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %3 : object, %6 : object
// CHECK-NEXT:  %8 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoShorthandMix2(func)
// CHECK-NEXT:frame = [__proto__, func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [__proto__]
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %2 = StoreFrameInst 42 : number, [__proto__]
// CHECK-NEXT:  %3 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, %3 : object
// CHECK-NEXT:  %5 = LoadFrameInst [__proto__]
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5, %4 : object, "__proto__" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
