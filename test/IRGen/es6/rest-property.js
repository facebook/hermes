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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [f1, f2, f3, f4, f5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %f1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %f2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %f3#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "f3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %f4#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "f4" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %f5#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "f5" : string
// CHECK-NEXT:  %11 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = LoadStackInst %11
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function f1#0#1(t)#2
// CHECK-NEXT:frame = [t#2, a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [t#2], %0
// CHECK-NEXT:  %4 = BinaryOperatorInst '==', %3, null : null
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.throwTypeError] : number, undefined : undefined, %3, "Cannot destructure 'undefined' or 'null'." : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %8 : object, %3, undefined : undefined
// CHECK-NEXT:  %10 = StoreFrameInst %9, [a#2], %0
// CHECK-NEXT:  %11 = LoadFrameInst [a#2], %0
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2#0#1(t)#3
// CHECK-NEXT:frame = [t#3, a#3, b#3, rest#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [b#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [rest#3], %0
// CHECK-NEXT:  %5 = LoadFrameInst [t#3], %0
// CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
// CHECK-NEXT:  %7 = StoreFrameInst %6, [a#3], %0
// CHECK-NEXT:  %8 = LoadPropertyInst %5, "b" : string
// CHECK-NEXT:  %9 = StoreFrameInst %8, [b#3], %0
// CHECK-NEXT:  %10 = AllocObjectLiteralInst "a" : string, 0 : number, "b" : string, 0 : number
// CHECK-NEXT:  %11 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %12 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %11 : object, %5, %10 : object
// CHECK-NEXT:  %13 = StoreFrameInst %12, [rest#3], %0
// CHECK-NEXT:  %14 = LoadFrameInst [rest#3], %0
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f3#0#1(t)#4
// CHECK-NEXT:frame = [t#4, a#4, rest#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f3#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %t, [t#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#4], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [rest#4], %0
// CHECK-NEXT:  %4 = LoadFrameInst [t#4], %0
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [a#4], %0
// CHECK-NEXT:  %7 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %8 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %8 : object, %4, %7 : object
// CHECK-NEXT:  %10 = StoreFrameInst %9, [rest#4], %0
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f4#0#1(o, t)#5
// CHECK-NEXT:frame = [o#5, t#5, a#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f4#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %t, [t#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [a#5], %0
// CHECK-NEXT:  %4 = LoadFrameInst [t#5], %0
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [a#5], %0
// CHECK-NEXT:  %7 = LoadFrameInst [o#5], %0
// CHECK-NEXT:  %8 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %9 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %9 : object, %4, %8 : object
// CHECK-NEXT:  %11 = StorePropertyInst %10, %7, "rest" : string
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f5#0#1(o)#6
// CHECK-NEXT:frame = [o#6, a#6, rest#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{f5#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %o, [o#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [a#6], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [rest#6], %0
// CHECK-NEXT:  %4 = LoadFrameInst [o#6], %0
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %6 = StoreFrameInst %5, [a#6], %0
// CHECK-NEXT:  %7 = LoadPropertyInst %4, "a" : string
// CHECK-NEXT:  %8 = StoreFrameInst %7, [a#6], %0
// CHECK-NEXT:  %9 = AllocObjectLiteralInst "a" : string, 0 : number
// CHECK-NEXT:  %10 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %10 : object, %4, %9 : object
// CHECK-NEXT:  %12 = StoreFrameInst %11, [rest#6], %0
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
