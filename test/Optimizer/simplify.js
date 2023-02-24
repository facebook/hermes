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

// CHECK:function global(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "add_sub_num": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "modulo_num": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "logic_ops_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "add_str": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "add_empty_str": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "add_empty_str_simplify": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "add_null": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "mul_null": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "left_shift_num": string
// CHECK-NEXT:  %9 = DeclareGlobalVarInst "left_shift_null": string
// CHECK-NEXT:  %10 = DeclareGlobalVarInst "left_shift_undefined": string
// CHECK-NEXT:  %11 = DeclareGlobalVarInst "right_shift_num": string
// CHECK-NEXT:  %12 = DeclareGlobalVarInst "right_shift_null": string
// CHECK-NEXT:  %13 = DeclareGlobalVarInst "right_shift_undefined": string
// CHECK-NEXT:  %14 = DeclareGlobalVarInst "right_shift_bool": string
// CHECK-NEXT:  %15 = DeclareGlobalVarInst "unsigned_right_shift_bool": string
// CHECK-NEXT:  %16 = DeclareGlobalVarInst "unsigned_right_shift_compound_assgmt": string
// CHECK-NEXT:  %17 = DeclareGlobalVarInst "unsigned_right_shift_num": string
// CHECK-NEXT:  %18 = DeclareGlobalVarInst "add_undef": string
// CHECK-NEXT:  %19 = DeclareGlobalVarInst "comp_num": string
// CHECK-NEXT:  %20 = DeclareGlobalVarInst "equality": string
// CHECK-NEXT:  %21 = DeclareGlobalVarInst "arith": string
// CHECK-NEXT:  %22 = DeclareGlobalVarInst "undef_test": string
// CHECK-NEXT:  %23 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %24 = DeclareGlobalVarInst "strip_bang": string
// CHECK-NEXT:  %25 = DeclareGlobalVarInst "turn_unary_plus_into_as_number": string
// CHECK-NEXT:  %26 = DeclareGlobalVarInst "turn_unary_plus_on_literal_into_result": string
// CHECK-NEXT:  %27 = DeclareGlobalVarInst "turn_bitor_into_as_int32": string
// CHECK-NEXT:  %28 = DeclareGlobalVarInst "unary_ops": string
// CHECK-NEXT:  %29 = DeclareGlobalVarInst "test_phi": string
// CHECK-NEXT:  %30 = DeclareGlobalVarInst "if_inline": string
// CHECK-NEXT:  %31 = DeclareGlobalVarInst "simplify_switch": string
// CHECK-NEXT:  %32 = DeclareGlobalVarInst "objectCond": string
// CHECK-NEXT:  %33 = CreateFunctionInst (:closure) %add_sub_num(): undefined
// CHECK-NEXT:  %34 = StorePropertyLooseInst %33: closure, globalObject: object, "add_sub_num": string
// CHECK-NEXT:  %35 = CreateFunctionInst (:closure) %modulo_num(): undefined
// CHECK-NEXT:  %36 = StorePropertyLooseInst %35: closure, globalObject: object, "modulo_num": string
// CHECK-NEXT:  %37 = CreateFunctionInst (:closure) %logic_ops_test(): undefined
// CHECK-NEXT:  %38 = StorePropertyLooseInst %37: closure, globalObject: object, "logic_ops_test": string
// CHECK-NEXT:  %39 = CreateFunctionInst (:closure) %add_str(): undefined
// CHECK-NEXT:  %40 = StorePropertyLooseInst %39: closure, globalObject: object, "add_str": string
// CHECK-NEXT:  %41 = CreateFunctionInst (:closure) %add_empty_str(): undefined
// CHECK-NEXT:  %42 = StorePropertyLooseInst %41: closure, globalObject: object, "add_empty_str": string
// CHECK-NEXT:  %43 = CreateFunctionInst (:closure) %add_empty_str_simplify(): string
// CHECK-NEXT:  %44 = StorePropertyLooseInst %43: closure, globalObject: object, "add_empty_str_simplify": string
// CHECK-NEXT:  %45 = CreateFunctionInst (:closure) %add_null(): undefined
// CHECK-NEXT:  %46 = StorePropertyLooseInst %45: closure, globalObject: object, "add_null": string
// CHECK-NEXT:  %47 = CreateFunctionInst (:closure) %mul_null(): undefined
// CHECK-NEXT:  %48 = StorePropertyLooseInst %47: closure, globalObject: object, "mul_null": string
// CHECK-NEXT:  %49 = CreateFunctionInst (:closure) %left_shift_num(): undefined
// CHECK-NEXT:  %50 = StorePropertyLooseInst %49: closure, globalObject: object, "left_shift_num": string
// CHECK-NEXT:  %51 = CreateFunctionInst (:closure) %left_shift_null(): undefined
// CHECK-NEXT:  %52 = StorePropertyLooseInst %51: closure, globalObject: object, "left_shift_null": string
// CHECK-NEXT:  %53 = CreateFunctionInst (:closure) %left_shift_undefined(): undefined
// CHECK-NEXT:  %54 = StorePropertyLooseInst %53: closure, globalObject: object, "left_shift_undefined": string
// CHECK-NEXT:  %55 = CreateFunctionInst (:closure) %right_shift_num(): undefined
// CHECK-NEXT:  %56 = StorePropertyLooseInst %55: closure, globalObject: object, "right_shift_num": string
// CHECK-NEXT:  %57 = CreateFunctionInst (:closure) %right_shift_null(): undefined
// CHECK-NEXT:  %58 = StorePropertyLooseInst %57: closure, globalObject: object, "right_shift_null": string
// CHECK-NEXT:  %59 = CreateFunctionInst (:closure) %right_shift_undefined(): undefined
// CHECK-NEXT:  %60 = StorePropertyLooseInst %59: closure, globalObject: object, "right_shift_undefined": string
// CHECK-NEXT:  %61 = CreateFunctionInst (:closure) %right_shift_bool(): undefined
// CHECK-NEXT:  %62 = StorePropertyLooseInst %61: closure, globalObject: object, "right_shift_bool": string
// CHECK-NEXT:  %63 = CreateFunctionInst (:closure) %unsigned_right_shift_bool(): undefined
// CHECK-NEXT:  %64 = StorePropertyLooseInst %63: closure, globalObject: object, "unsigned_right_shift_bool": string
// CHECK-NEXT:  %65 = CreateFunctionInst (:closure) %unsigned_right_shift_compound_assgmt(): undefined
// CHECK-NEXT:  %66 = StorePropertyLooseInst %65: closure, globalObject: object, "unsigned_right_shift_compound_assgmt": string
// CHECK-NEXT:  %67 = CreateFunctionInst (:closure) %unsigned_right_shift_num(): undefined
// CHECK-NEXT:  %68 = StorePropertyLooseInst %67: closure, globalObject: object, "unsigned_right_shift_num": string
// CHECK-NEXT:  %69 = CreateFunctionInst (:closure) %add_undef(): undefined
// CHECK-NEXT:  %70 = StorePropertyLooseInst %69: closure, globalObject: object, "add_undef": string
// CHECK-NEXT:  %71 = CreateFunctionInst (:closure) %comp_num(): undefined
// CHECK-NEXT:  %72 = StorePropertyLooseInst %71: closure, globalObject: object, "comp_num": string
// CHECK-NEXT:  %73 = CreateFunctionInst (:closure) %equality(): undefined
// CHECK-NEXT:  %74 = StorePropertyLooseInst %73: closure, globalObject: object, "equality": string
// CHECK-NEXT:  %75 = CreateFunctionInst (:closure) %arith(): undefined
// CHECK-NEXT:  %76 = StorePropertyLooseInst %75: closure, globalObject: object, "arith": string
// CHECK-NEXT:  %77 = CreateFunctionInst (:closure) %undef_test(): undefined
// CHECK-NEXT:  %78 = StorePropertyLooseInst %77: closure, globalObject: object, "undef_test": string
// CHECK-NEXT:  %79 = CreateFunctionInst (:closure) %foo(): number
// CHECK-NEXT:  %80 = StorePropertyLooseInst %79: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %81 = CreateFunctionInst (:closure) %strip_bang(): number
// CHECK-NEXT:  %82 = StorePropertyLooseInst %81: closure, globalObject: object, "strip_bang": string
// CHECK-NEXT:  %83 = CreateFunctionInst (:closure) %turn_unary_plus_into_as_number(): number
// CHECK-NEXT:  %84 = StorePropertyLooseInst %83: closure, globalObject: object, "turn_unary_plus_into_as_number": string
// CHECK-NEXT:  %85 = CreateFunctionInst (:closure) %turn_unary_plus_on_literal_into_result(): number
// CHECK-NEXT:  %86 = StorePropertyLooseInst %85: closure, globalObject: object, "turn_unary_plus_on_literal_into_result": string
// CHECK-NEXT:  %87 = CreateFunctionInst (:closure) %turn_bitor_into_as_int32(): number
// CHECK-NEXT:  %88 = StorePropertyLooseInst %87: closure, globalObject: object, "turn_bitor_into_as_int32": string
// CHECK-NEXT:  %89 = CreateFunctionInst (:closure) %unary_ops(): undefined
// CHECK-NEXT:  %90 = StorePropertyLooseInst %89: closure, globalObject: object, "unary_ops": string
// CHECK-NEXT:  %91 = CreateFunctionInst (:closure) %test_phi(): number
// CHECK-NEXT:  %92 = StorePropertyLooseInst %91: closure, globalObject: object, "test_phi": string
// CHECK-NEXT:  %93 = CreateFunctionInst (:closure) %if_inline(): number
// CHECK-NEXT:  %94 = StorePropertyLooseInst %93: closure, globalObject: object, "if_inline": string
// CHECK-NEXT:  %95 = CreateFunctionInst (:closure) %simplify_switch(): undefined
// CHECK-NEXT:  %96 = StorePropertyLooseInst %95: closure, globalObject: object, "simplify_switch": string
// CHECK-NEXT:  %97 = CreateFunctionInst (:closure) %objectCond(): number
// CHECK-NEXT:  %98 = StorePropertyLooseInst %97: closure, globalObject: object, "objectCond": string
// CHECK-NEXT:  %99 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_sub_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 133: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1.5: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -7.5: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, Infinity: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -0: number
// CHECK-NEXT:  %14 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -Infinity: number
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %17 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %18 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %19 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function modulo_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -2: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0.5: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0.5: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %14 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function logic_ops_test(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 23: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 20: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -727379959: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 99: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -2: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 9: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 17: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 23: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 17: number
// CHECK-NEXT:  %13 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 23: number
// CHECK-NEXT:  %14 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %15 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %16 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %17 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_str(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "hello world": string
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "hello NaN": string
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "NaNworld": string
// CHECK-NEXT:  %4 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %2 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, %2: string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %5 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %6 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, %5: string
// CHECK-NEXT:  %7 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_empty_str_simplify(x: any): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %2 = ReturnInst (:string) %1: string
// CHECK-NEXT:function_end

// CHECK:function add_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "nullhello": string
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "hellonull": string
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function mul_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 8: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 10: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 32: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -2: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %8 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function left_shift_undefined(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -4: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %11 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_null(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_undefined(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 5: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function right_shift_bool(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %9 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_bool(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_compound_assgmt(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 1: number
// CHECK-NEXT:  %2 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function unsigned_right_shift_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 3: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 134217727: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 31: number
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 536870908: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4294967295: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %11 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function add_undef(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "undefinedasdf": string
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "asdfundefined": string
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function comp_num(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %6 = AddEmptyStringInst (:string) %0: any
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %8 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %9 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %10 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %11 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %13 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %14 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %15 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %16 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %17 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %18 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %19 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function equality(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %5 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %6 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %7 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %8 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %9 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %10 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %11 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %13 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %14 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %15 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %16 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %17 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %18 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, false: boolean
// CHECK-NEXT:  %19 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function arith(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 4: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 8: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 64: number
// CHECK-NEXT:  %4 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function undef_test(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = BinaryGreaterThanOrEqualInst (:boolean) undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, %1: boolean
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %4 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = ReturnInst (:number) 8: number
// CHECK-NEXT:function_end

// CHECK:function strip_bang(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %2 = CondBranchInst %1: number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %3 = ReturnInst (:number) 1: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst (:number) 2: number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_into_as_number(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsNumberInst (:number) %0: any
// CHECK-NEXT:  %2 = ReturnInst (:number) %1: number
// CHECK-NEXT:function_end

// CHECK:function turn_unary_plus_on_literal_into_result(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 5: number
// CHECK-NEXT:function_end

// CHECK:function turn_bitor_into_as_int32(y: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = AsInt32Inst (:number) %0: any
// CHECK-NEXT:  %2 = ReturnInst (:number) %1: number
// CHECK-NEXT:function_end

// CHECK:function unary_ops(x: any, y: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "number": string
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "object": string
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "string": string
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "boolean": string
// CHECK-NEXT:  %5 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "object": string
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, "function": string
// CHECK-NEXT:  %7 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -9: number
// CHECK-NEXT:  %8 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -1: number
// CHECK-NEXT:  %9 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -0: number
// CHECK-NEXT:  %10 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, -0: number
// CHECK-NEXT:  %11 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, NaN: number
// CHECK-NEXT:  %12 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, true: boolean
// CHECK-NEXT:  %13 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_phi(a: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 4: number
// CHECK-NEXT:function_end

// CHECK:function if_inline(d: any): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = ReturnInst (:number) 10: number
// CHECK-NEXT:function_end

// CHECK:function simplify_switch(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 7: number
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %4 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function objectCond(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 1: number
// CHECK-NEXT:function_end
