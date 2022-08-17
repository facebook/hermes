/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s


//CHECK-LABEL:function add_sub_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 133 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 1.5 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -7.5 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 2 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 2 : number
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, Infinity : number
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, -0 : number
//CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, -Infinity : number
//CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %18 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %19 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function add_sub_num(x, y) {
  var sink = print;

  // Some constants:
  sink(1 + 0)
  sink(1 - 0)

  // Multiplication:
  sink(19 * 7)

  // Add/Sub constants:
  sink(1 + 2)
  sink(2.5 - 1.0)
  sink(2.5 - 1.5)
  sink(2.5 - 10)
  sink(2.5 + 1.5)

  //Division operations with consts
  sink(48 / 24)
  sink(4.8 / 2.4)
  sink(4 / 0)
  sink(0 / 0)
  sink(-0 / 4)
  sink(10 / -0)

  // Operations with NaN - should all eval to NaN
  sink(2.5 - (undefined + 1))
  sink((undefined + 1) + 1.5)
  sink(19 * (undefined + 1))
  sink((undefined + 1) / -0)
}

//CHECK-LABEL:function modulo_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 2 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, -2 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 2 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0.5 : number
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, 0.5 : number
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %14 = ReturnInst undefined : undefined
function modulo_num(x, y) {
  var sink = print;

  // Integer and float constants
  sink(6 % 5)
  sink(2 % 3)
  sink(-2 % 3)
  sink(2 % -3)
  sink(10 % 0)
  sink(5.5 % 1.5)
  sink(5.5 % -1.5)
  sink(-5.5 % 1.5)
  sink(-5.5 % -1.5)
  sink(5.5 % 2.5)
  sink(5.5 % -2.5)

  // Operations with NaN - should all eval to NaN
  sink(10 % (undefined + 1))
  sink((undefined + 1) % 12)
}

//CHECK-LABEL:function logic_ops_test(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 23 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 20 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -727379959 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 99 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, -2 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 9 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 17 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 23 : number
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, 17 : number
//CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, 23 : number
//CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %17 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function logic_ops_test(x, y) {
  var sink = print;

  // Arithmetic operations:
  sink(19 & 7)
  sink(19 | 7)
  sink(19 ^ 7)

  sink(9 ^ 1000000000000)
  sink(1 | 6871947673699)
  sink(-2 & -2)
  sink(9 & 9.1)

  sink(19 & (undefined+1))
  sink(17 | (undefined+1))
  sink(23 ^ (undefined+1))
  sink((undefined+1) & 19)
  sink((undefined+1) | 17)
  sink((undefined+1) ^ 23)
  sink((undefined+1) & (undefined+1))
  sink((undefined+1) | (undefined+1))
  sink((undefined+1) ^ (undefined+1))
}

//CHECK-LABEL:function add_str(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "hello world" : string
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, "hello NaN" : string
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "NaNworld" : string
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function add_str(x, y) {
  var sink = print;
  sink("hello " + "world")
  sink("hello " + (undefined+1))
  sink((undefined+1) + "world")
}

//CHECK-LABEL:function add_empty_str(x)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:  %1 = AddEmptyStringInst %x
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : string
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:  %4 = AddEmptyStringInst %x
//CHECK-NEXT:  %5 = CallInst %3, undefined : undefined, %4 : string
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function add_empty_str(x) {
  sink("" + x);
  sink(x + "");
}

//CHECK-LABEL:function add_empty_str_simplify(x) : string
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AddEmptyStringInst %x
//CHECK-NEXT:  %1 = ReturnInst %0 : string
//CHECK-NEXT:function_end
function add_empty_str_simplify(x) {
    x = x + "";
    return "" + x;
}

//CHECK-LABEL:function add_null(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "nullhello" : string
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "hellonull" : string
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function add_null(x, y) {
  var sink = print;

  sink(null + 3)
  sink(3 + null)

  sink(null + "hello")
  sink("hello" + null)

  sink(null + null)
}

//CHECK-LABEL:function mul_null(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, -0 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function mul_null(x, y) {
  var sink = print;

  sink(null * 3)
  sink(3 * null)
  sink(null * -3)
  sink(-3 * null)

  sink(null * null)
}

//CHECK-LABEL:function left_shift_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 8 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 10 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 32 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -2 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %8 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function left_shift_num(x, y) {
  var sink = print;

  sink(1 << 3)
  sink(5 << 1)
  sink(2 << 4)
  sink(-1 << 1)
  sink(3 << (undefined+1))
  sink((undefined+1) << 3)
  sink((undefined+1) << (undefined+1))
}

//CHECK-LABEL:function left_shift_null(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function left_shift_null(x, y) {
  var sink = print;

  sink(null << 3)
  sink(5 << null)
  sink(null << null)
  sink(null << (undefined+1))
  sink((undefined+1) << null)
}

//CHECK-LABEL:function left_shift_undefined(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function left_shift_undefined(x, y) {
  var sink = print;

  sink(undefined << 3)
  sink(5 << undefined)
  sink(undefined << undefined)
  sink(undefined << (undefined+1))
  sink((undefined+1) << undefined)
}

//CHECK-LABEL:function right_shift_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -4 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function right_shift_num(x, y) {
  var sink = print;

  sink(9 >> 1)
  sink(8 >> 1)
  sink(27 >> 3)
  sink(1 >> 3)

  sink(-1 >> 5)
  sink(-1 >> -5)
  sink(-27 >> 3)

  sink((undefined+1) >> 5)
  sink(-1 >> (undefined+1))
  sink((undefined+1) >> (undefined+1))
}

//CHECK-LABEL:function right_shift_null(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function right_shift_null(x, y) {
  var sink = print;

  sink(null >> 3)
  sink(5 >> null)
  sink(null >> null)
  sink(null >> (undefined+1))
  sink((undefined+1) >> null)
}

//CHECK-LABEL:function right_shift_undefined(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function right_shift_undefined(x, y) {
  var sink = print;

  sink(undefined >> 3)
  sink(5 >> undefined)
  sink(undefined >> undefined)
  sink(undefined >> (undefined+1))
  sink((undefined+1) >> undefined)
}

//CHECK-LABEL:function right_shift_bool(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %9 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function right_shift_bool(x, y) {
  var sink = print;

  sink(true >> 1)
  sink(1 >> true)
  sink(true >> null)
  sink(null >> true)
  sink(true >> undefined)
  sink(undefined >> true)
  sink(true >> (undefined+1))
  sink((undefined+1) >> true)
}

//CHECK-LABEL:function unsigned_right_shift_bool(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function unsigned_right_shift_bool(x, y) {
  var sink = print;

  sink(true >>> null)
  sink(null >>> true)
  sink(true >>> (undefined+1))
  sink((undefined+1) >>> true)
}

//CHECK-LABEL:function unsigned_right_shift_compound_assgmt(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function unsigned_right_shift_compound_assgmt(x, y) {
  var sink = print;
  var x = true;
  x >>>= null;

  sink(x)
}

//CHECK-LABEL:function unsigned_right_shift_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 3 : number
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 134217727 : number
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 31 : number
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 536870908 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 4294967295 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function unsigned_right_shift_num(x, y) {
  var sink = print;

  sink(9 >>> 1)
  sink(8 >>> 1)
  sink(27 >>> 3)
  sink(1 >>> 3)

  sink(-1 >>> 5)
  sink(-1 >>> -5)
  sink(-27 >>> 3)

  sink(-1 >>> (undefined+1))
  sink((undefined+1) >>> -5)
  sink((undefined+1) >>> (undefined+1))
}

//CHECK-LABEL:function add_undef(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "undefinedasdf" : string
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "asdfundefined" : string
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function add_undef(x, y) {
  var sink = print;

  sink(undefined + 3)
  sink(3 + undefined)

  sink(undefined + "asdf")
  sink("asdf" + undefined)

  sink(undefined + undefined)
}

//CHECK-LABEL:function comp_num(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %5 = AddEmptyStringInst %y
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %18 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function comp_num(x, y) {
  var sink = print;
  sink(2.5 > 1.5)
  sink(2.5 < 1.5)
  sink(2.5 >= 1.5)
  sink(2.5 <= 1.5)

  // Make x a string. We can't make it an integer because we currently
  // can't differentiate between integer vs a number that can be NaN.
  var x = "" + y;

  sink(x >  x)
  sink(x <  x)
  sink(x >= x)
  sink(x <= x)

  sink("x" >  "x")
  sink("x" <  "x")
  sink("x" >= "x")
  sink("x" <= "x")

  sink(5 < (undefined+1))
  sink((undefined+1) <= 5)
  sink("x" > (undefined+1))
  sink((undefined+1) > "x")
}

//CHECK-LABEL:function equality(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %9 = AsInt32Inst %y
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, false : boolean
//CHECK-NEXT:  %18 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function equality(x, y) {
  var sink = print;
  sink(2.5 === 1.5)
  sink(1.5 === 1.5)
  sink("11" === "22")
  sink("11" === "11")

  sink(2.5 !== 1.5)
  sink(1.5 !== 1.5)
  sink("11" !== "22")
  sink("11" !== "11")

  // Make x an integer.
  var x = y | 0;

  sink(null === x)
  sink(11 === null)

  sink(2.5 === (undefined+1))
  sink((undefined+1) === "22")
  sink((undefined+1) !== 1.5)
  sink("11" !== (undefined+1))
  sink((undefined+1) !== (undefined+1))
  sink((undefined+1) === (undefined+1))
}

//CHECK-LABEL:function arith()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 8 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 64 : number
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function arith() {
  var sink = print;
  sink(2 * 2)
  sink(2 * 4)
  sink(2 ** 6)
}


//CHECK-LABEL:function undef_test(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = BinaryOperatorInst '>=', undefined : undefined, undefined : undefined
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : boolean
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function undef_test(x, y) {
  var sink = print;
  sink(undefined >= undefined)
  sink(undefined == undefined)
}

//CHECK-LABEL:function foo(y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = ReturnInst 8 : number
//CHECK-NEXT:function_end
function foo(y) {
  var sink = print;
  var y = 5;
  y++;
  y++;
  y++;
  return y;
}

//CHECK-LABEL:function strip_bang(y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AsInt32Inst %y
//CHECK-NEXT:  %1 = CondBranchInst %0 : number, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %2 = ReturnInst 1 : number
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = ReturnInst 2 : number
//CHECK-NEXT:function_end
function strip_bang(y) {
  var x = y | 0;
  if (!x) {
    return 1
  } else {
    return 2
  }
}

//CHECK-LABEL:function turn_unary_plus_into_as_number(y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AsNumberInst %y
//CHECK-NEXT:  %1 = ReturnInst %0
//CHECK-NEXT:function_end
function turn_unary_plus_into_as_number(y) {
  return +y;
}

//CHECK-LABEL:function turn_unary_plus_on_literal_into_result()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 5
//CHECK-NEXT:function_end
function turn_unary_plus_on_literal_into_result() {
  return +5;
}

//CHECK-LABEL:function turn_bitor_into_as_int32(y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AsInt32Inst %y
//CHECK-NEXT:  %1 = ReturnInst %0 : number
//CHECK-NEXT:function_end
function turn_bitor_into_as_int32(y) {
  return y | 0;
}

//CHECK-LABEL:function unary_ops(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "number" : string
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, "object" : string
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "string" : string
//CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "boolean" : string
//CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, "object" : string
//CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, "function" : string
//CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -9 : number
//CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, -1 : number
//CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -0 : number
//CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, -0 : number
//CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, NaN : number
//CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, true : boolean
//CHECK-NEXT:  %13 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function unary_ops(x, y) {
  var sink = print;

  sink(typeof(1))
  sink(typeof(null))
  sink(typeof("gi"))
  sink(typeof(true))
  sink(typeof(/a+b/))
  sink(typeof(function (){}))
  sink(-9)
  sink(-true)
  sink(-false)
  sink(-null)
  sink(-undefined)
  sink(! (x ? null : undefined))
}

// Make sure that we are removing the PHI node.
//CHECK-LABEL:function test_phi(a)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 4 : number
//CHECK-NEXT:function_end
function test_phi(a) {
  var x = 0;
  if (a) {
    x = 4;
  } else {
    x = 4;
  }
  return x;
}

//CHECK-LABEL:function if_inline(d)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = ReturnInst 10 : number
//CHECK-NEXT:function_end
function if_inline(d)
{
  var sink = print;

  var c = 20;
  if (c > 10) {
    c = 10;
  }

  return c;
}

//CHECK-LABEL:function simplify_switch()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 7 : number
//CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
//CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 2 : number
//CHECK-NEXT:  %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function simplify_switch()
{
  var sink = print;

  var i = 0;
  switch (i) {
    case 3:
      sink(3);
      break;
    default:
      sink(7);
      break;
  }

  switch (0) {
    case 0:
      sink(0);
    default:
      sink(2);
  }

  switch (4) {
    case 5:
      sink(5);
      break;
    case "4":
      sink(4);
      break;
  }
}

//CHECK-LABEL:function objectCond() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst 1 : number
//CHECK-NEXT:function_end
function objectCond() {
  if ({}) return 1;
  return 2;
}
