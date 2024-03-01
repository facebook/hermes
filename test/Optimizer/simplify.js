/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

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
  sink(undefined != undefined)
  sink(undefined == null)
  sink(null == undefined)
  sink(undefined != null)
  sink(null != undefined)
  sink(undefined == 0)
  sink(null == 0)
  sink("" == null)
  sink("" == undefined)
  sink(undefined == false)
  sink(null == false)
  sink(true == undefined)
  sink(true == null)
  sink(undefined != 0)
  sink(null != 0)
  sink("" != undefined)
  sink("" != null)
  sink(undefined != false)
  sink(null != false)
  sink(true != undefined)
  sink(true != null)
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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "add_sub_num": string
// CHECK-NEXT:       DeclareGlobalVarInst "modulo_num": string
// CHECK-NEXT:       DeclareGlobalVarInst "logic_ops_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "add_str": string
// CHECK-NEXT:       DeclareGlobalVarInst "add_empty_str": string
// CHECK-NEXT:       DeclareGlobalVarInst "add_empty_str_simplify": string
// CHECK-NEXT:       DeclareGlobalVarInst "add_null": string
// CHECK-NEXT:       DeclareGlobalVarInst "mul_null": string
// CHECK-NEXT:       DeclareGlobalVarInst "left_shift_num": string
// CHECK-NEXT:        DeclareGlobalVarInst "left_shift_null": string
// CHECK-NEXT:        DeclareGlobalVarInst "left_shift_undefined": string
// CHECK-NEXT:        DeclareGlobalVarInst "right_shift_num": string
// CHECK-NEXT:        DeclareGlobalVarInst "right_shift_null": string
// CHECK-NEXT:        DeclareGlobalVarInst "right_shift_undefined": string
// CHECK-NEXT:        DeclareGlobalVarInst "right_shift_bool": string
// CHECK-NEXT:        DeclareGlobalVarInst "unsigned_right_shift_bool": string
// CHECK-NEXT:        DeclareGlobalVarInst "unsigned_right_shift_compound_assgmt": string
// CHECK-NEXT:        DeclareGlobalVarInst "unsigned_right_shift_num": string
// CHECK-NEXT:        DeclareGlobalVarInst "add_undef": string
// CHECK-NEXT:        DeclareGlobalVarInst "comp_num": string
// CHECK-NEXT:        DeclareGlobalVarInst "equality": string
// CHECK-NEXT:        DeclareGlobalVarInst "arith": string
// CHECK-NEXT:        DeclareGlobalVarInst "undef_test": string
// CHECK-NEXT:        DeclareGlobalVarInst "foo": string
// CHECK-NEXT:        DeclareGlobalVarInst "strip_bang": string
// CHECK-NEXT:        DeclareGlobalVarInst "turn_unary_plus_into_as_number": string
// CHECK-NEXT:        DeclareGlobalVarInst "turn_unary_plus_on_literal_into_result": string
// CHECK-NEXT:        DeclareGlobalVarInst "turn_bitor_into_as_int32": string
// CHECK-NEXT:        DeclareGlobalVarInst "unary_ops": string
// CHECK-NEXT:        DeclareGlobalVarInst "test_phi": string
// CHECK-NEXT:        DeclareGlobalVarInst "if_inline": string
// CHECK-NEXT:        DeclareGlobalVarInst "simplify_switch": string
// CHECK-NEXT:        DeclareGlobalVarInst "objectCond": string
// CHECK-NEXT:  %34 = CreateFunctionInst (:object) %0: environment, %add_sub_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %34: object, globalObject: object, "add_sub_num": string
// CHECK-NEXT:  %36 = CreateFunctionInst (:object) %0: environment, %modulo_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %36: object, globalObject: object, "modulo_num": string
// CHECK-NEXT:  %38 = CreateFunctionInst (:object) %0: environment, %logic_ops_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %38: object, globalObject: object, "logic_ops_test": string
// CHECK-NEXT:  %40 = CreateFunctionInst (:object) %0: environment, %add_str(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %40: object, globalObject: object, "add_str": string
// CHECK-NEXT:  %42 = CreateFunctionInst (:object) %0: environment, %add_empty_str(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %42: object, globalObject: object, "add_empty_str": string
// CHECK-NEXT:  %44 = CreateFunctionInst (:object) %0: environment, %add_empty_str_simplify(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %44: object, globalObject: object, "add_empty_str_simplify": string
// CHECK-NEXT:  %46 = CreateFunctionInst (:object) %0: environment, %add_null(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %46: object, globalObject: object, "add_null": string
// CHECK-NEXT:  %48 = CreateFunctionInst (:object) %0: environment, %mul_null(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %48: object, globalObject: object, "mul_null": string
// CHECK-NEXT:  %50 = CreateFunctionInst (:object) %0: environment, %left_shift_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %50: object, globalObject: object, "left_shift_num": string
// CHECK-NEXT:  %52 = CreateFunctionInst (:object) %0: environment, %left_shift_null(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %52: object, globalObject: object, "left_shift_null": string
// CHECK-NEXT:  %54 = CreateFunctionInst (:object) %0: environment, %left_shift_undefined(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %54: object, globalObject: object, "left_shift_undefined": string
// CHECK-NEXT:  %56 = CreateFunctionInst (:object) %0: environment, %right_shift_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %56: object, globalObject: object, "right_shift_num": string
// CHECK-NEXT:  %58 = CreateFunctionInst (:object) %0: environment, %right_shift_null(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %58: object, globalObject: object, "right_shift_null": string
// CHECK-NEXT:  %60 = CreateFunctionInst (:object) %0: environment, %right_shift_undefined(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %60: object, globalObject: object, "right_shift_undefined": string
// CHECK-NEXT:  %62 = CreateFunctionInst (:object) %0: environment, %right_shift_bool(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %62: object, globalObject: object, "right_shift_bool": string
// CHECK-NEXT:  %64 = CreateFunctionInst (:object) %0: environment, %unsigned_right_shift_bool(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %64: object, globalObject: object, "unsigned_right_shift_bool": string
// CHECK-NEXT:  %66 = CreateFunctionInst (:object) %0: environment, %unsigned_right_shift_compound_assgmt(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %66: object, globalObject: object, "unsigned_right_shift_compound_assgmt": string
// CHECK-NEXT:  %68 = CreateFunctionInst (:object) %0: environment, %unsigned_right_shift_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %68: object, globalObject: object, "unsigned_right_shift_num": string
// CHECK-NEXT:  %70 = CreateFunctionInst (:object) %0: environment, %add_undef(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %70: object, globalObject: object, "add_undef": string
// CHECK-NEXT:  %72 = CreateFunctionInst (:object) %0: environment, %comp_num(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %72: object, globalObject: object, "comp_num": string
// CHECK-NEXT:  %74 = CreateFunctionInst (:object) %0: environment, %equality(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %74: object, globalObject: object, "equality": string
// CHECK-NEXT:  %76 = CreateFunctionInst (:object) %0: environment, %arith(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %76: object, globalObject: object, "arith": string
// CHECK-NEXT:  %78 = CreateFunctionInst (:object) %0: environment, %undef_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %78: object, globalObject: object, "undef_test": string
// CHECK-NEXT:  %80 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %80: object, globalObject: object, "foo": string
// CHECK-NEXT:  %82 = CreateFunctionInst (:object) %0: environment, %strip_bang(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %82: object, globalObject: object, "strip_bang": string
// CHECK-NEXT:  %84 = CreateFunctionInst (:object) %0: environment, %turn_unary_plus_into_as_number(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %84: object, globalObject: object, "turn_unary_plus_into_as_number": string
// CHECK-NEXT:  %86 = CreateFunctionInst (:object) %0: environment, %turn_unary_plus_on_literal_into_result(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %86: object, globalObject: object, "turn_unary_plus_on_literal_into_result": string
// CHECK-NEXT:  %88 = CreateFunctionInst (:object) %0: environment, %turn_bitor_into_as_int32(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %88: object, globalObject: object, "turn_bitor_into_as_int32": string
// CHECK-NEXT:  %90 = CreateFunctionInst (:object) %0: environment, %unary_ops(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %90: object, globalObject: object, "unary_ops": string
// CHECK-NEXT:  %92 = CreateFunctionInst (:object) %0: environment, %test_phi(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %92: object, globalObject: object, "test_phi": string
// CHECK-NEXT:  %94 = CreateFunctionInst (:object) %0: environment, %if_inline(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %94: object, globalObject: object, "if_inline": string
// CHECK-NEXT:  %96 = CreateFunctionInst (:object) %0: environment, %simplify_switch(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %96: object, globalObject: object, "simplify_switch": string
// CHECK-NEXT:  %98 = CreateFunctionInst (:object) %0: environment, %objectCond(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %98: object, globalObject: object, "objectCond": string
// CHECK-NEXT:         ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_sub_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 133: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1.5: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -7.5: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, Infinity: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -0: number
// CHECK-NEXT:  %14 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -Infinity: number
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %17 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %18 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function modulo_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -2: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0.5: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0.5: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function logic_ops_test(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 23: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 20: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -727379959: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 99: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -2: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 9: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 23: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 17: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 23: number
// CHECK-NEXT:  %14 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_str(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "hello world": string
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "hello NaN": string
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "NaNworld": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %2 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %5 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %6 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str_simplify(x: any): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:       ReturnInst %1: string
// CHECK-NEXT:function_end

// CHECK:function add_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "nullhello": string
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "hellonull": string
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function mul_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 8: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 10: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 32: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -2: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_undefined(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -4: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_undefined(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_bool(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_bool(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_compound_assgmt(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 1: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 3: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 134217727: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 31: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 536870908: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4294967295: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_undef(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "undefinedasdf": string
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "asdfundefined": string
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function comp_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %6 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %8 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %9 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %10 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %11 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %13 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %14 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %15 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %16 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %17 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %18 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function equality(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %6 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %8 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %9 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %10 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %11 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %13 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %14 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %15 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %16 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %17 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %18 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function arith(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 8: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 64: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function undef_test(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = BinaryGreaterThanOrEqualInst (:boolean) undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %14 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %17 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %18 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %19 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %20 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %21 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %22 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %23 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %24 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:       ReturnInst 8: number
// CHECK-NEXT:function_end

// CHECK:function strip_bang(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:       CondBranchInst %1: number, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_into_as_number(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsNumberInst (:number) %0: any
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_on_literal_into_result(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 5: number
// CHECK-NEXT:function_end

// CHECK:function turn_bitor_into_as_int32(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:function unary_ops(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %unary_ops(): any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "number": string
// CHECK-NEXT:  %4 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "object": string
// CHECK-NEXT:  %5 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "string": string
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "boolean": string
// CHECK-NEXT:  %7 = CreateRegExpInst (:object) "a+b": string, "": string
// CHECK-NEXT:  %8 = UnaryTypeofInst (:string) %7: object
// CHECK-NEXT:  %9 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:  %11 = UnaryTypeofInst (:string) %10: object
// CHECK-NEXT:  %12 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: string
// CHECK-NEXT:  %13 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -9: number
// CHECK-NEXT:  %14 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -1: number
// CHECK-NEXT:  %15 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -0: number
// CHECK-NEXT:  %16 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, -0: number
// CHECK-NEXT:  %17 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, NaN: number
// CHECK-NEXT:  %18 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_phi(a: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 4: number
// CHECK-NEXT:function_end

// CHECK:function if_inline(d: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:       ReturnInst 10: number
// CHECK-NEXT:function_end

// CHECK:function simplify_switch(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 7: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function objectCond(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function ""(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
