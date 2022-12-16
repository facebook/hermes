/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(t) {
    var {...a} = t;
    return a;
}

function f2(t) {
    var {a, b, ...rest} = t;
    return rest;
}

function f3(t) {
    var a, rest;
    ({a, ...rest} = t);
}

function f4(o, t) {
    var a;
    ({a, ...o.rest} = t);
}

function f5(o) {
    var a, rest;
    ({a, a,  ...rest} = o);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [f1, f2, f3, f4, f5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %f1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %f2()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %f3()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "f3" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %f4()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "f4" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %f5()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "f5" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = LoadStackInst %10
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function f1(t)
// CHECK-NEXT:frame = [a, t]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = LoadParamInst %t
// CHECK-NEXT:  %2 = StoreFrameInst %1, [t]
// CHECK-NEXT:  %3 = LoadFrameInst [t]
// CHECK-NEXT:  %4 = BinaryOperatorInst '==', %3, null : null
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, %3, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %8 : object, %3, undefined : undefined
// CHECK-NEXT:  %10 = StoreFrameInst %9, [a]
// CHECK-NEXT:  %11 = LoadFrameInst [a]
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2(t)
// CHECK-NEXT:frame = [a, b, rest, t]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [b]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [rest]
// CHECK-NEXT:  %3 = LoadParamInst %t
// CHECK-NEXT:  %4 = StoreFrameInst %3, [t]
// CHECK-NEXT:  %5 = LoadFrameInst [t]
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
// CHECK-NEXT:  %7 = StoreFrameInst %6, [a]
// CHECK-NEXT:  %8 = LoadPropertyInst %5, "b" : string
// CHECK-NEXT:  %9 = StoreFrameInst %8, [b]
// CHECK-NEXT:  %10 = AllocObjectLiteralInst "a" : string, 0 : number, "b" : string, 0 : number
// CHECK-NEXT:  %11 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %11 : object, %5, %10 : object
// CHECK-NEXT:  %13 = StoreFrameInst %12, [rest]
// CHECK-NEXT:  %14 = LoadFrameInst [rest]
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f3(t)
// CHECK-NEXT:frame = [a, rest, t]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [rest]
// CHECK-NEXT:  %2 = LoadParamInst %t
// CHECK-NEXT:  %3 = StoreFrameInst %2, [t]
// CHECK-NEXT:  %4 = LoadFrameInst [t]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [a]
// CHECK-NEXT:  %7 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %8 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %8 : object, %4, %7 : object
// CHECK-NEXT:  %10 = StoreFrameInst %9, [rest]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f4(o, t)
// CHECK-NEXT:frame = [a, o, t]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = LoadParamInst %o
// CHECK-NEXT:  %2 = StoreFrameInst %1, [o]
// CHECK-NEXT:  %3 = LoadParamInst %t
// CHECK-NEXT:  %4 = StoreFrameInst %3, [t]
// CHECK-NEXT:  %5 = LoadFrameInst [t]
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
// CHECK-NEXT:  %7 = StoreFrameInst %6, [a]
// CHECK-NEXT:  %8 = LoadFrameInst [o]
// CHECK-NEXT:  %9 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %10 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %10 : object, %5, %9 : object
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11, %8, "rest" : string
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f5(o)
// CHECK-NEXT:frame = [a, rest, o]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [a]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [rest]
// CHECK-NEXT:  %2 = LoadParamInst %o
// CHECK-NEXT:  %3 = StoreFrameInst %2, [o]
// CHECK-NEXT:  %4 = LoadFrameInst [o]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [a]
// CHECK-NEXT:  %7 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %8 = StoreFrameInst %7, [a]
// CHECK-NEXT:  %9 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %10 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %10 : object, %4, %9 : object
// CHECK-NEXT:  %12 = StoreFrameInst %11, [rest]
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
