/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

  function foo(x, y) {
    x = y;
    x += y;
    x -= y;
    x *= y;
    x /= y;
    x %= y;
    //x **= y; Not ES5.
    x <<= y;
    x >>= y;
    x >>>= y;
    x &= y;
    x ^= y;
    x |= y;

    x.t = y;
    x.t += y;
    x.t -= y;
    x.t *= y;
    x.t /= y;
    x.t %= y;
    //x.t **= y; Not ES5.
    x.t <<= y;
    x.t >>= y;
    x.t >>>= y;
    x.t &= y;
    x.t ^= y;
    x.t |= y;

    return x == y;
    return x != y;
    return x === y;
    // return x !=== y; Not ES5.
    return x < y;
    return x <= y;
    return x > y;
    return x >= y;
    return x << y;
    return x << y;
    return x >>> y;
    return x + y;
    return x - y;
    return x * y;
    return x / y;
    return x % y;
    return x | y;
    return x ^ y;
    return x & y;
    return x in y;
    return x instanceof y;
  }

  function assignment_test(x, y) {
    x = y;
    x += y;
  }

  function member_test(x, y) {
    x.t += y;
  }

  function binary_ops(x, y) {
    return x >>> y;
  }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "assignment_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "member_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "binary_ops": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %assignment_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "assignment_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %member_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "member_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %binary_ops(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "binary_ops": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       StoreFrameInst %8: any, [x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %12 = BinarySubtractInst (:any) %10: any, %11: any
// CHECK-NEXT:        StoreFrameInst %12: any, [x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %16 = BinaryMultiplyInst (:any) %14: any, %15: any
// CHECK-NEXT:        StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %20 = BinaryDivideInst (:any) %18: any, %19: any
// CHECK-NEXT:        StoreFrameInst %20: any, [x]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %24 = BinaryModuloInst (:any) %22: any, %23: any
// CHECK-NEXT:        StoreFrameInst %24: any, [x]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %28 = BinaryLeftShiftInst (:any) %26: any, %27: any
// CHECK-NEXT:        StoreFrameInst %28: any, [x]: any
// CHECK-NEXT:  %30 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %32 = BinaryRightShiftInst (:any) %30: any, %31: any
// CHECK-NEXT:        StoreFrameInst %32: any, [x]: any
// CHECK-NEXT:  %34 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %35 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %36 = BinaryUnsignedRightShiftInst (:any) %34: any, %35: any
// CHECK-NEXT:        StoreFrameInst %36: any, [x]: any
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %40 = BinaryAndInst (:any) %38: any, %39: any
// CHECK-NEXT:        StoreFrameInst %40: any, [x]: any
// CHECK-NEXT:  %42 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %43 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %44 = BinaryXorInst (:any) %42: any, %43: any
// CHECK-NEXT:        StoreFrameInst %44: any, [x]: any
// CHECK-NEXT:  %46 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %47 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %48 = BinaryOrInst (:any) %46: any, %47: any
// CHECK-NEXT:        StoreFrameInst %48: any, [x]: any
// CHECK-NEXT:  %50 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %51 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %51: any, %50: any, "t": string
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %54 = LoadPropertyInst (:any) %53: any, "t": string
// CHECK-NEXT:  %55 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %56 = BinaryAddInst (:any) %54: any, %55: any
// CHECK-NEXT:        StorePropertyLooseInst %56: any, %53: any, "t": string
// CHECK-NEXT:  %58 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %59 = LoadPropertyInst (:any) %58: any, "t": string
// CHECK-NEXT:  %60 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %61 = BinarySubtractInst (:any) %59: any, %60: any
// CHECK-NEXT:        StorePropertyLooseInst %61: any, %58: any, "t": string
// CHECK-NEXT:  %63 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %64 = LoadPropertyInst (:any) %63: any, "t": string
// CHECK-NEXT:  %65 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %66 = BinaryMultiplyInst (:any) %64: any, %65: any
// CHECK-NEXT:        StorePropertyLooseInst %66: any, %63: any, "t": string
// CHECK-NEXT:  %68 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %69 = LoadPropertyInst (:any) %68: any, "t": string
// CHECK-NEXT:  %70 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %71 = BinaryDivideInst (:any) %69: any, %70: any
// CHECK-NEXT:        StorePropertyLooseInst %71: any, %68: any, "t": string
// CHECK-NEXT:  %73 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %74 = LoadPropertyInst (:any) %73: any, "t": string
// CHECK-NEXT:  %75 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %76 = BinaryModuloInst (:any) %74: any, %75: any
// CHECK-NEXT:        StorePropertyLooseInst %76: any, %73: any, "t": string
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %79 = LoadPropertyInst (:any) %78: any, "t": string
// CHECK-NEXT:  %80 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %81 = BinaryLeftShiftInst (:any) %79: any, %80: any
// CHECK-NEXT:        StorePropertyLooseInst %81: any, %78: any, "t": string
// CHECK-NEXT:  %83 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %84 = LoadPropertyInst (:any) %83: any, "t": string
// CHECK-NEXT:  %85 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %86 = BinaryRightShiftInst (:any) %84: any, %85: any
// CHECK-NEXT:        StorePropertyLooseInst %86: any, %83: any, "t": string
// CHECK-NEXT:  %88 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %89 = LoadPropertyInst (:any) %88: any, "t": string
// CHECK-NEXT:  %90 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %91 = BinaryUnsignedRightShiftInst (:any) %89: any, %90: any
// CHECK-NEXT:        StorePropertyLooseInst %91: any, %88: any, "t": string
// CHECK-NEXT:  %93 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %94 = LoadPropertyInst (:any) %93: any, "t": string
// CHECK-NEXT:  %95 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %96 = BinaryAndInst (:any) %94: any, %95: any
// CHECK-NEXT:        StorePropertyLooseInst %96: any, %93: any, "t": string
// CHECK-NEXT:  %98 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %99 = LoadPropertyInst (:any) %98: any, "t": string
// CHECK-NEXT:  %100 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %101 = BinaryXorInst (:any) %99: any, %100: any
// CHECK-NEXT:         StorePropertyLooseInst %101: any, %98: any, "t": string
// CHECK-NEXT:  %103 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %104 = LoadPropertyInst (:any) %103: any, "t": string
// CHECK-NEXT:  %105 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %106 = BinaryOrInst (:any) %104: any, %105: any
// CHECK-NEXT:         StorePropertyLooseInst %106: any, %103: any, "t": string
// CHECK-NEXT:  %108 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %109 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %110 = BinaryEqualInst (:boolean) %108: any, %109: any
// CHECK-NEXT:         ReturnInst %110: boolean
// CHECK-NEXT:function_end

// CHECK:function assignment_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:       StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:       StoreFrameInst %8: any, [x]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function member_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "t": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any
// CHECK-NEXT:       StorePropertyLooseInst %7: any, %4: any, "t": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryUnsignedRightShiftInst (:any) %4: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end
