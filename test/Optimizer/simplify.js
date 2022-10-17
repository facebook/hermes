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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [add_sub_num, modulo_num, logic_ops_test, add_str, add_empty_str, add_empty_str_simplify, add_null, mul_null, left_shift_num, left_shift_null, left_shift_undefined, right_shift_num, right_shift_null, right_shift_undefined, right_shift_bool, unsigned_right_shift_bool, unsigned_right_shift_compound_assgmt, unsigned_right_shift_num, add_undef, comp_num, equality, arith, undef_test, foo, strip_bang, turn_unary_plus_into_as_number, turn_unary_plus_on_literal_into_result, turn_bitor_into_as_int32, unary_ops, test_phi, if_inline, simplify_switch, objectCond]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %add_sub_num#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "add_sub_num" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %modulo_num#0#1()#3 : undefined, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "modulo_num" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %logic_ops_test#0#1()#4 : undefined, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "logic_ops_test" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %add_str#0#1()#5 : undefined, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "add_str" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %add_empty_str#0#1()#6 : undefined, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "add_empty_str" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %add_empty_str_simplify#0#1()#7 : string, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "add_empty_str_simplify" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %add_null#0#1()#8 : undefined, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "add_null" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %mul_null#0#1()#9 : undefined, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "mul_null" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %left_shift_num#0#1()#10 : undefined, %0
// CHECK-NEXT:  %18 = StorePropertyInst %17 : closure, globalObject : object, "left_shift_num" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %left_shift_null#0#1()#11 : undefined, %0
// CHECK-NEXT:  %20 = StorePropertyInst %19 : closure, globalObject : object, "left_shift_null" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %left_shift_undefined#0#1()#12 : undefined, %0
// CHECK-NEXT:  %22 = StorePropertyInst %21 : closure, globalObject : object, "left_shift_undefined" : string
// CHECK-NEXT:  %23 = CreateFunctionInst %right_shift_num#0#1()#13 : undefined, %0
// CHECK-NEXT:  %24 = StorePropertyInst %23 : closure, globalObject : object, "right_shift_num" : string
// CHECK-NEXT:  %25 = CreateFunctionInst %right_shift_null#0#1()#14 : undefined, %0
// CHECK-NEXT:  %26 = StorePropertyInst %25 : closure, globalObject : object, "right_shift_null" : string
// CHECK-NEXT:  %27 = CreateFunctionInst %right_shift_undefined#0#1()#15 : undefined, %0
// CHECK-NEXT:  %28 = StorePropertyInst %27 : closure, globalObject : object, "right_shift_undefined" : string
// CHECK-NEXT:  %29 = CreateFunctionInst %right_shift_bool#0#1()#16 : undefined, %0
// CHECK-NEXT:  %30 = StorePropertyInst %29 : closure, globalObject : object, "right_shift_bool" : string
// CHECK-NEXT:  %31 = CreateFunctionInst %unsigned_right_shift_bool#0#1()#17 : undefined, %0
// CHECK-NEXT:  %32 = StorePropertyInst %31 : closure, globalObject : object, "unsigned_right_shift_bool" : string
// CHECK-NEXT:  %33 = CreateFunctionInst %unsigned_right_shift_compound_assgmt#0#1()#18 : undefined, %0
// CHECK-NEXT:  %34 = StorePropertyInst %33 : closure, globalObject : object, "unsigned_right_shift_compound_assgmt" : string
// CHECK-NEXT:  %35 = CreateFunctionInst %unsigned_right_shift_num#0#1()#19 : undefined, %0
// CHECK-NEXT:  %36 = StorePropertyInst %35 : closure, globalObject : object, "unsigned_right_shift_num" : string
// CHECK-NEXT:  %37 = CreateFunctionInst %add_undef#0#1()#20 : undefined, %0
// CHECK-NEXT:  %38 = StorePropertyInst %37 : closure, globalObject : object, "add_undef" : string
// CHECK-NEXT:  %39 = CreateFunctionInst %comp_num#0#1()#21 : undefined, %0
// CHECK-NEXT:  %40 = StorePropertyInst %39 : closure, globalObject : object, "comp_num" : string
// CHECK-NEXT:  %41 = CreateFunctionInst %equality#0#1()#22 : undefined, %0
// CHECK-NEXT:  %42 = StorePropertyInst %41 : closure, globalObject : object, "equality" : string
// CHECK-NEXT:  %43 = CreateFunctionInst %arith#0#1()#23 : undefined, %0
// CHECK-NEXT:  %44 = StorePropertyInst %43 : closure, globalObject : object, "arith" : string
// CHECK-NEXT:  %45 = CreateFunctionInst %undef_test#0#1()#24 : undefined, %0
// CHECK-NEXT:  %46 = StorePropertyInst %45 : closure, globalObject : object, "undef_test" : string
// CHECK-NEXT:  %47 = CreateFunctionInst %foo#0#1()#25 : number, %0
// CHECK-NEXT:  %48 = StorePropertyInst %47 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %49 = CreateFunctionInst %strip_bang#0#1()#26 : number, %0
// CHECK-NEXT:  %50 = StorePropertyInst %49 : closure, globalObject : object, "strip_bang" : string
// CHECK-NEXT:  %51 = CreateFunctionInst %turn_unary_plus_into_as_number#0#1()#27 : number, %0
// CHECK-NEXT:  %52 = StorePropertyInst %51 : closure, globalObject : object, "turn_unary_plus_into_as_number" : string
// CHECK-NEXT:  %53 = CreateFunctionInst %turn_unary_plus_on_literal_into_result#0#1()#28 : number, %0
// CHECK-NEXT:  %54 = StorePropertyInst %53 : closure, globalObject : object, "turn_unary_plus_on_literal_into_result" : string
// CHECK-NEXT:  %55 = CreateFunctionInst %turn_bitor_into_as_int32#0#1()#29 : number, %0
// CHECK-NEXT:  %56 = StorePropertyInst %55 : closure, globalObject : object, "turn_bitor_into_as_int32" : string
// CHECK-NEXT:  %57 = CreateFunctionInst %unary_ops#0#1()#30 : undefined, %0
// CHECK-NEXT:  %58 = StorePropertyInst %57 : closure, globalObject : object, "unary_ops" : string
// CHECK-NEXT:  %59 = CreateFunctionInst %test_phi#0#1()#32 : number, %0
// CHECK-NEXT:  %60 = StorePropertyInst %59 : closure, globalObject : object, "test_phi" : string
// CHECK-NEXT:  %61 = CreateFunctionInst %if_inline#0#1()#33 : number, %0
// CHECK-NEXT:  %62 = StorePropertyInst %61 : closure, globalObject : object, "if_inline" : string
// CHECK-NEXT:  %63 = CreateFunctionInst %simplify_switch#0#1()#34 : undefined, %0
// CHECK-NEXT:  %64 = StorePropertyInst %63 : closure, globalObject : object, "simplify_switch" : string
// CHECK-NEXT:  %65 = CreateFunctionInst %objectCond#0#1()#35 : number, %0
// CHECK-NEXT:  %66 = StorePropertyInst %65 : closure, globalObject : object, "objectCond" : string
// CHECK-NEXT:  %67 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_sub_num#0#1(x, y)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_sub_num#0#1()#2}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 133 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 1.5 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, -7.5 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, 2 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, 2 : number
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, Infinity : number
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %14 = CallInst %1, undefined : undefined, -0 : number
// CHECK-NEXT:  %15 = CallInst %1, undefined : undefined, -Infinity : number
// CHECK-NEXT:  %16 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %17 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %18 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %19 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function modulo_num#0#1(x, y)#3 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{modulo_num#0#1()#3}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 2 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, -2 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 2 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, 0.5 : number
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, 0.5 : number
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %14 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %15 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function logic_ops_test#0#1(x, y)#4 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{logic_ops_test#0#1()#4}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 23 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 20 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, -727379959 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 99 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, -2 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, 9 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, 17 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, 23 : number
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, 17 : number
// CHECK-NEXT:  %14 = CallInst %1, undefined : undefined, 23 : number
// CHECK-NEXT:  %15 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %16 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %17 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_str#0#1(x, y)#5 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_str#0#1()#5}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "hello world" : string
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, "hello NaN" : string
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, "NaNworld" : string
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str#0#1(x)#6 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_empty_str#0#1()#6}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = AddEmptyStringInst %x
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, %2 : string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %5 = AddEmptyStringInst %x
// CHECK-NEXT:  %6 = CallInst %4, undefined : undefined, %5 : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str_simplify#0#1(x)#7 : string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_empty_str_simplify#0#1()#7}
// CHECK-NEXT:  %1 = AddEmptyStringInst %x
// CHECK-NEXT:  %2 = ReturnInst %1 : string
// CHECK-NEXT:function_end

// CHECK:function add_null#0#1(x, y)#8 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_null#0#1()#8}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, "nullhello" : string
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, "hellonull" : string
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function mul_null#0#1(x, y)#9 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{mul_null#0#1()#9}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, -0 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, -0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_num#0#1(x, y)#10 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{left_shift_num#0#1()#10}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 8 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 10 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 32 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, -2 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_null#0#1(x, y)#11 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{left_shift_null#0#1()#11}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 5 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_undefined#0#1(x, y)#12 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{left_shift_undefined#0#1()#12}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 5 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_num#0#1(x, y)#13 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{right_shift_num#0#1()#13}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, -4 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_null#0#1(x, y)#14 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{right_shift_null#0#1()#14}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 5 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_undefined#0#1(x, y)#15 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{right_shift_undefined#0#1()#15}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 5 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_bool#0#1(x, y)#16 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{right_shift_bool#0#1()#16}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_bool#0#1(x, y)#17 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{unsigned_right_shift_bool#0#1()#17}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_compound_assgmt#0#1(x, y)#18 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{unsigned_right_shift_compound_assgmt#0#1()#18}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_num#0#1(x, y)#19 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{unsigned_right_shift_num#0#1()#19}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 3 : number
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, 134217727 : number
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, 31 : number
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, 536870908 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, 4294967295 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function add_undef#0#1(x, y)#20 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{add_undef#0#1()#20}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, "undefinedasdf" : string
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, "asdfundefined" : string
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function comp_num#0#1(x, y)#21 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{comp_num#0#1()#21}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %6 = AddEmptyStringInst %y
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %14 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %15 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %16 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %17 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %18 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function equality#0#1(x, y)#22 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{equality#0#1()#22}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %10 = AsInt32Inst %y
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %14 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %15 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %16 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %17 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %18 = CallInst %1, undefined : undefined, false : boolean
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function arith#0#1()#23 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{arith#0#1()#23}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 4 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 8 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 64 : number
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function undef_test#0#1(x, y)#24 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{undef_test#0#1()#24}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '>=', undefined : undefined, undefined : undefined
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, %2 : boolean
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(y)#25 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#25}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = ReturnInst 8 : number
// CHECK-NEXT:function_end

// CHECK:function strip_bang#0#1(y)#26 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{strip_bang#0#1()#26}
// CHECK-NEXT:  %1 = AsInt32Inst %y
// CHECK-NEXT:  %2 = CondBranchInst %1 : number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst 2 : number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_into_as_number#0#1(y)#27 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{turn_unary_plus_into_as_number#0#1()#27}
// CHECK-NEXT:  %1 = AsNumberInst %y
// CHECK-NEXT:  %2 = ReturnInst %1 : number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_on_literal_into_result#0#1()#28 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{turn_unary_plus_on_literal_into_result#0#1()#28}
// CHECK-NEXT:  %1 = ReturnInst 5 : number
// CHECK-NEXT:function_end

// CHECK:function turn_bitor_into_as_int32#0#1(y)#29 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{turn_bitor_into_as_int32#0#1()#29}
// CHECK-NEXT:  %1 = AsInt32Inst %y
// CHECK-NEXT:  %2 = ReturnInst %1 : number
// CHECK-NEXT:function_end

// CHECK:function unary_ops#0#1(x, y)#30 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{unary_ops#0#1()#30}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, "number" : string
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, "object" : string
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, "string" : string
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, "boolean" : string
// CHECK-NEXT:  %6 = CallInst %1, undefined : undefined, "object" : string
// CHECK-NEXT:  %7 = CallInst %1, undefined : undefined, "function" : string
// CHECK-NEXT:  %8 = CallInst %1, undefined : undefined, -9 : number
// CHECK-NEXT:  %9 = CallInst %1, undefined : undefined, -1 : number
// CHECK-NEXT:  %10 = CallInst %1, undefined : undefined, -0 : number
// CHECK-NEXT:  %11 = CallInst %1, undefined : undefined, -0 : number
// CHECK-NEXT:  %12 = CallInst %1, undefined : undefined, NaN : number
// CHECK-NEXT:  %13 = CallInst %1, undefined : undefined, true : boolean
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_phi#0#1(a)#32 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_phi#0#1()#32}
// CHECK-NEXT:  %1 = ReturnInst 4 : number
// CHECK-NEXT:function_end

// CHECK:function if_inline#0#1(d)#33 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{if_inline#0#1()#33}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = ReturnInst 10 : number
// CHECK-NEXT:function_end

// CHECK:function simplify_switch#0#1()#34 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simplify_switch#0#1()#34}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, 7 : number
// CHECK-NEXT:  %3 = CallInst %1, undefined : undefined, 0 : number
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, 2 : number
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function objectCond#0#1()#35 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{objectCond#0#1()#35}
// CHECK-NEXT:  %1 = ReturnInst 1 : number
// CHECK-NEXT:function_end
