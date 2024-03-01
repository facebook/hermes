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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "assignment_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "member_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "binary_ops": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "foo": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %assignment_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "assignment_test": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %member_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "member_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %binary_ops(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "binary_ops": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [x]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %10 = BinaryAddInst (:any) %8: any, %9: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [x]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %14 = BinarySubtractInst (:any) %12: any, %13: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [x]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %18 = BinaryMultiplyInst (:any) %16: any, %17: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [x]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %22 = BinaryDivideInst (:any) %20: any, %21: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: any, [x]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %25 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %26 = BinaryModuloInst (:any) %24: any, %25: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: any, [x]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %30 = BinaryLeftShiftInst (:any) %28: any, %29: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: any, [x]: any
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %33 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %34 = BinaryRightShiftInst (:any) %32: any, %33: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: any, [x]: any
// CHECK-NEXT:  %36 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %37 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %38 = BinaryUnsignedRightShiftInst (:any) %36: any, %37: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %38: any, [x]: any
// CHECK-NEXT:  %40 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %41 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %42 = BinaryAndInst (:any) %40: any, %41: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %42: any, [x]: any
// CHECK-NEXT:  %44 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %45 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %46 = BinaryXorInst (:any) %44: any, %45: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %46: any, [x]: any
// CHECK-NEXT:  %48 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %49 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %50 = BinaryOrInst (:any) %48: any, %49: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %50: any, [x]: any
// CHECK-NEXT:  %52 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %53 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %53: any, %52: any, "t": string
// CHECK-NEXT:  %55 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %56 = LoadPropertyInst (:any) %55: any, "t": string
// CHECK-NEXT:  %57 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %58 = BinaryAddInst (:any) %56: any, %57: any
// CHECK-NEXT:        StorePropertyLooseInst %58: any, %55: any, "t": string
// CHECK-NEXT:  %60 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %61 = LoadPropertyInst (:any) %60: any, "t": string
// CHECK-NEXT:  %62 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %63 = BinarySubtractInst (:any) %61: any, %62: any
// CHECK-NEXT:        StorePropertyLooseInst %63: any, %60: any, "t": string
// CHECK-NEXT:  %65 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %66 = LoadPropertyInst (:any) %65: any, "t": string
// CHECK-NEXT:  %67 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %68 = BinaryMultiplyInst (:any) %66: any, %67: any
// CHECK-NEXT:        StorePropertyLooseInst %68: any, %65: any, "t": string
// CHECK-NEXT:  %70 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %71 = LoadPropertyInst (:any) %70: any, "t": string
// CHECK-NEXT:  %72 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %73 = BinaryDivideInst (:any) %71: any, %72: any
// CHECK-NEXT:        StorePropertyLooseInst %73: any, %70: any, "t": string
// CHECK-NEXT:  %75 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %76 = LoadPropertyInst (:any) %75: any, "t": string
// CHECK-NEXT:  %77 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %78 = BinaryModuloInst (:any) %76: any, %77: any
// CHECK-NEXT:        StorePropertyLooseInst %78: any, %75: any, "t": string
// CHECK-NEXT:  %80 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %81 = LoadPropertyInst (:any) %80: any, "t": string
// CHECK-NEXT:  %82 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %83 = BinaryLeftShiftInst (:any) %81: any, %82: any
// CHECK-NEXT:        StorePropertyLooseInst %83: any, %80: any, "t": string
// CHECK-NEXT:  %85 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %86 = LoadPropertyInst (:any) %85: any, "t": string
// CHECK-NEXT:  %87 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %88 = BinaryRightShiftInst (:any) %86: any, %87: any
// CHECK-NEXT:        StorePropertyLooseInst %88: any, %85: any, "t": string
// CHECK-NEXT:  %90 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %91 = LoadPropertyInst (:any) %90: any, "t": string
// CHECK-NEXT:  %92 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %93 = BinaryUnsignedRightShiftInst (:any) %91: any, %92: any
// CHECK-NEXT:        StorePropertyLooseInst %93: any, %90: any, "t": string
// CHECK-NEXT:  %95 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %96 = LoadPropertyInst (:any) %95: any, "t": string
// CHECK-NEXT:  %97 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %98 = BinaryAndInst (:any) %96: any, %97: any
// CHECK-NEXT:        StorePropertyLooseInst %98: any, %95: any, "t": string
// CHECK-NEXT:  %100 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %101 = LoadPropertyInst (:any) %100: any, "t": string
// CHECK-NEXT:  %102 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %103 = BinaryXorInst (:any) %101: any, %102: any
// CHECK-NEXT:         StorePropertyLooseInst %103: any, %100: any, "t": string
// CHECK-NEXT:  %105 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %106 = LoadPropertyInst (:any) %105: any, "t": string
// CHECK-NEXT:  %107 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %108 = BinaryOrInst (:any) %106: any, %107: any
// CHECK-NEXT:         StorePropertyLooseInst %108: any, %105: any, "t": string
// CHECK-NEXT:  %110 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %111 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %112 = BinaryEqualInst (:boolean) %110: any, %111: any
// CHECK-NEXT:         ReturnInst %112: boolean
// CHECK-NEXT:function_end

// CHECK:function assignment_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %assignment_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [x]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %10 = BinaryAddInst (:any) %8: any, %9: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [x]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function member_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %member_test(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "t": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any
// CHECK-NEXT:        StorePropertyLooseInst %9: any, %6: any, "t": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %binary_ops(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %8 = BinaryUnsignedRightShiftInst (:any) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end
