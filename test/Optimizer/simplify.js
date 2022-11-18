/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

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

function add_str(x, y) {
  var sink = print;
  sink("hello " + "world")
  sink("hello " + (undefined+1))
  sink((undefined+1) + "world")
}

function add_empty_str(x) {
  sink("" + x);
  sink(x + "");
}

function add_empty_str_simplify(x) {
    x = x + "";
    return "" + x;
}

function add_null(x, y) {
  var sink = print;

  sink(null + 3)
  sink(3 + null)

  sink(null + "hello")
  sink("hello" + null)

  sink(null + null)
}

function mul_null(x, y) {
  var sink = print;

  sink(null * 3)
  sink(3 * null)
  sink(null * -3)
  sink(-3 * null)

  sink(null * null)
}

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

function left_shift_null(x, y) {
  var sink = print;

  sink(null << 3)
  sink(5 << null)
  sink(null << null)
  sink(null << (undefined+1))
  sink((undefined+1) << null)
}

function left_shift_undefined(x, y) {
  var sink = print;

  sink(undefined << 3)
  sink(5 << undefined)
  sink(undefined << undefined)
  sink(undefined << (undefined+1))
  sink((undefined+1) << undefined)
}

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

function right_shift_null(x, y) {
  var sink = print;

  sink(null >> 3)
  sink(5 >> null)
  sink(null >> null)
  sink(null >> (undefined+1))
  sink((undefined+1) >> null)
}

function right_shift_undefined(x, y) {
  var sink = print;

  sink(undefined >> 3)
  sink(5 >> undefined)
  sink(undefined >> undefined)
  sink(undefined >> (undefined+1))
  sink((undefined+1) >> undefined)
}

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

function unsigned_right_shift_bool(x, y) {
  var sink = print;

  sink(true >>> null)
  sink(null >>> true)
  sink(true >>> (undefined+1))
  sink((undefined+1) >>> true)
}

function unsigned_right_shift_compound_assgmt(x, y) {
  var sink = print;
  var x = true;
  x >>>= null;

  sink(x)
}

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

function add_undef(x, y) {
  var sink = print;

  sink(undefined + 3)
  sink(3 + undefined)

  sink(undefined + "asdf")
  sink("asdf" + undefined)

  sink(undefined + undefined)
}

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

function arith() {
  var sink = print;
  sink(2 * 2)
  sink(2 * 4)
  sink(2 ** 6)
}

function undef_test(x, y) {
  var sink = print;
  sink(undefined >= undefined)
  sink(undefined == undefined)
}

function foo(y) {
  var sink = print;
  var y = 5;
  y++;
  y++;
  y++;
  return y;
}

function strip_bang(y) {
  var x = y | 0;
  if (!x) {
    return 1
  } else {
    return 2
  }
}

function turn_unary_plus_into_as_number(y) {
  return +y;
}

function turn_unary_plus_on_literal_into_result() {
  return +5;
}

function turn_bitor_into_as_int32(y) {
  return y | 0;
}

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
function test_phi(a) {
  var x = 0;
  if (a) {
    x = 4;
  } else {
    x = 4;
  }
  return x;
}

function if_inline(d)
{
  var sink = print;

  var c = 20;
  if (c > 10) {
    c = 10;
  }

  return c;
}

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

function objectCond() {
  if ({}) return 1;
  return 2;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [add_sub_num, modulo_num, logic_ops_test, add_str, add_empty_str, add_empty_str_simplify, add_null, mul_null, left_shift_num, left_shift_null, left_shift_undefined, right_shift_num, right_shift_null, right_shift_undefined, right_shift_bool, unsigned_right_shift_bool, unsigned_right_shift_compound_assgmt, unsigned_right_shift_num, add_undef, comp_num, equality, arith, undef_test, foo, strip_bang, turn_unary_plus_into_as_number, turn_unary_plus_on_literal_into_result, turn_bitor_into_as_int32, unary_ops, test_phi, if_inline, simplify_switch, objectCond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %add_sub_num() : undefined
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "add_sub_num" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %modulo_num() : undefined
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "modulo_num" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %logic_ops_test() : undefined
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "logic_ops_test" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %add_str() : undefined
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "add_str" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %add_empty_str() : undefined
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "add_empty_str" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %add_empty_str_simplify() : string
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "add_empty_str_simplify" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %add_null() : undefined
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "add_null" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %mul_null() : undefined
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "mul_null" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %left_shift_num() : undefined
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "left_shift_num" : string
// CHECK-NEXT:  %18 = CreateFunctionInst %left_shift_null() : undefined
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18 : closure, globalObject : object, "left_shift_null" : string
// CHECK-NEXT:  %20 = CreateFunctionInst %left_shift_undefined() : undefined
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20 : closure, globalObject : object, "left_shift_undefined" : string
// CHECK-NEXT:  %22 = CreateFunctionInst %right_shift_num() : undefined
// CHECK-NEXT:  %23 = StorePropertyLooseInst %22 : closure, globalObject : object, "right_shift_num" : string
// CHECK-NEXT:  %24 = CreateFunctionInst %right_shift_null() : undefined
// CHECK-NEXT:  %25 = StorePropertyLooseInst %24 : closure, globalObject : object, "right_shift_null" : string
// CHECK-NEXT:  %26 = CreateFunctionInst %right_shift_undefined() : undefined
// CHECK-NEXT:  %27 = StorePropertyLooseInst %26 : closure, globalObject : object, "right_shift_undefined" : string
// CHECK-NEXT:  %28 = CreateFunctionInst %right_shift_bool() : undefined
// CHECK-NEXT:  %29 = StorePropertyLooseInst %28 : closure, globalObject : object, "right_shift_bool" : string
// CHECK-NEXT:  %30 = CreateFunctionInst %unsigned_right_shift_bool() : undefined
// CHECK-NEXT:  %31 = StorePropertyLooseInst %30 : closure, globalObject : object, "unsigned_right_shift_bool" : string
// CHECK-NEXT:  %32 = CreateFunctionInst %unsigned_right_shift_compound_assgmt() : undefined
// CHECK-NEXT:  %33 = StorePropertyLooseInst %32 : closure, globalObject : object, "unsigned_right_shift_compound_assgmt" : string
// CHECK-NEXT:  %34 = CreateFunctionInst %unsigned_right_shift_num() : undefined
// CHECK-NEXT:  %35 = StorePropertyLooseInst %34 : closure, globalObject : object, "unsigned_right_shift_num" : string
// CHECK-NEXT:  %36 = CreateFunctionInst %add_undef() : undefined
// CHECK-NEXT:  %37 = StorePropertyLooseInst %36 : closure, globalObject : object, "add_undef" : string
// CHECK-NEXT:  %38 = CreateFunctionInst %comp_num() : undefined
// CHECK-NEXT:  %39 = StorePropertyLooseInst %38 : closure, globalObject : object, "comp_num" : string
// CHECK-NEXT:  %40 = CreateFunctionInst %equality() : undefined
// CHECK-NEXT:  %41 = StorePropertyLooseInst %40 : closure, globalObject : object, "equality" : string
// CHECK-NEXT:  %42 = CreateFunctionInst %arith() : undefined
// CHECK-NEXT:  %43 = StorePropertyLooseInst %42 : closure, globalObject : object, "arith" : string
// CHECK-NEXT:  %44 = CreateFunctionInst %undef_test() : undefined
// CHECK-NEXT:  %45 = StorePropertyLooseInst %44 : closure, globalObject : object, "undef_test" : string
// CHECK-NEXT:  %46 = CreateFunctionInst %foo() : number
// CHECK-NEXT:  %47 = StorePropertyLooseInst %46 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %48 = CreateFunctionInst %strip_bang() : number
// CHECK-NEXT:  %49 = StorePropertyLooseInst %48 : closure, globalObject : object, "strip_bang" : string
// CHECK-NEXT:  %50 = CreateFunctionInst %turn_unary_plus_into_as_number() : number
// CHECK-NEXT:  %51 = StorePropertyLooseInst %50 : closure, globalObject : object, "turn_unary_plus_into_as_number" : string
// CHECK-NEXT:  %52 = CreateFunctionInst %turn_unary_plus_on_literal_into_result() : number
// CHECK-NEXT:  %53 = StorePropertyLooseInst %52 : closure, globalObject : object, "turn_unary_plus_on_literal_into_result" : string
// CHECK-NEXT:  %54 = CreateFunctionInst %turn_bitor_into_as_int32() : number
// CHECK-NEXT:  %55 = StorePropertyLooseInst %54 : closure, globalObject : object, "turn_bitor_into_as_int32" : string
// CHECK-NEXT:  %56 = CreateFunctionInst %unary_ops() : undefined
// CHECK-NEXT:  %57 = StorePropertyLooseInst %56 : closure, globalObject : object, "unary_ops" : string
// CHECK-NEXT:  %58 = CreateFunctionInst %test_phi() : number
// CHECK-NEXT:  %59 = StorePropertyLooseInst %58 : closure, globalObject : object, "test_phi" : string
// CHECK-NEXT:  %60 = CreateFunctionInst %if_inline() : number
// CHECK-NEXT:  %61 = StorePropertyLooseInst %60 : closure, globalObject : object, "if_inline" : string
// CHECK-NEXT:  %62 = CreateFunctionInst %simplify_switch() : undefined
// CHECK-NEXT:  %63 = StorePropertyLooseInst %62 : closure, globalObject : object, "simplify_switch" : string
// CHECK-NEXT:  %64 = CreateFunctionInst %objectCond() : number
// CHECK-NEXT:  %65 = StorePropertyLooseInst %64 : closure, globalObject : object, "objectCond" : string
// CHECK-NEXT:  %66 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_sub_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 133 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 1.5 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -7.5 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 2 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 2 : number
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, Infinity : number
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, -0 : number
// CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, -Infinity : number
// CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %18 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modulo_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 2 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, -2 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 2 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0.5 : number
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, 0.5 : number
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logic_ops_test(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 23 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 20 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -727379959 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 99 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, -2 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 9 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 17 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 23 : number
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, 17 : number
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, 23 : number
// CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_str(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "hello world" : string
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, "hello NaN" : string
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "NaNworld" : string
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str(x) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %1 = AddEmptyStringInst %x
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : string
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %4 = AddEmptyStringInst %x
// CHECK-NEXT:  %5 = CallInst %3, undefined : undefined, %4 : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str_simplify(x) : string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AddEmptyStringInst %x
// CHECK-NEXT:  %1 = ReturnInst %0 : string
// CHECK-NEXT:function_end

// CHECK:function add_null(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "nullhello" : string
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "hellonull" : string
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function mul_null(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, -0 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 8 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 10 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 32 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, -2 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_null(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_undefined(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -4 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_null(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_undefined(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 5 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_bool(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_bool(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_compound_assgmt(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 3 : number
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, 134217727 : number
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, 31 : number
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, 536870908 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, 4294967295 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_undef(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "undefinedasdf" : string
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "asdfundefined" : string
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function comp_num(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %5 = AddEmptyStringInst %y
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function equality(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %9 = AsInt32Inst %y
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %13 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %14 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %15 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %16 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %17 = CallInst %0, undefined : undefined, false : boolean
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function arith() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 4 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 8 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 64 : number
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function undef_test(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '>=', undefined : undefined, undefined : undefined
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, %1 : boolean
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(y) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = ReturnInst 8 : number
// CHECK-NEXT:function_end

// CHECK:function strip_bang(y) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AsInt32Inst %y
// CHECK-NEXT:  %1 = CondBranchInst %0 : number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst 2 : number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_into_as_number(y) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AsNumberInst %y
// CHECK-NEXT:  %1 = ReturnInst %0 : number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_on_literal_into_result() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 5 : number
// CHECK-NEXT:function_end

// CHECK:function turn_bitor_into_as_int32(y) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AsInt32Inst %y
// CHECK-NEXT:  %1 = ReturnInst %0 : number
// CHECK-NEXT:function_end

// CHECK:function unary_ops(x, y) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, "number" : string
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, "object" : string
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, "string" : string
// CHECK-NEXT:  %4 = CallInst %0, undefined : undefined, "boolean" : string
// CHECK-NEXT:  %5 = CallInst %0, undefined : undefined, "object" : string
// CHECK-NEXT:  %6 = CallInst %0, undefined : undefined, "function" : string
// CHECK-NEXT:  %7 = CallInst %0, undefined : undefined, -9 : number
// CHECK-NEXT:  %8 = CallInst %0, undefined : undefined, -1 : number
// CHECK-NEXT:  %9 = CallInst %0, undefined : undefined, -0 : number
// CHECK-NEXT:  %10 = CallInst %0, undefined : undefined, -0 : number
// CHECK-NEXT:  %11 = CallInst %0, undefined : undefined, NaN : number
// CHECK-NEXT:  %12 = CallInst %0, undefined : undefined, true : boolean
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_phi(a) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 4 : number
// CHECK-NEXT:function_end

// CHECK:function if_inline(d) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = ReturnInst 10 : number
// CHECK-NEXT:function_end

// CHECK:function simplify_switch() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, 7 : number
// CHECK-NEXT:  %2 = CallInst %0, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %0, undefined : undefined, 2 : number
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function objectCond() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1 : number
// CHECK-NEXT:function_end
