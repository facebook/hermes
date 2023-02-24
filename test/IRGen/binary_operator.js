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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "assignment_test": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "member_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "binary_ops": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %assignment_test(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: closure, globalObject: object, "assignment_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %member_test(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: closure, globalObject: object, "member_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %binary_ops(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: closure, globalObject: object, "binary_ops": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:  %9 = StoreFrameInst %8: any, [x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %12 = BinarySubtractInst (:any) %10: any, %11: any
// CHECK-NEXT:  %13 = StoreFrameInst %12: any, [x]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %16 = BinaryMultiplyInst (:any) %14: any, %15: any
// CHECK-NEXT:  %17 = StoreFrameInst %16: any, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %20 = BinaryDivideInst (:any) %18: any, %19: any
// CHECK-NEXT:  %21 = StoreFrameInst %20: any, [x]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %24 = BinaryModuloInst (:any) %22: any, %23: any
// CHECK-NEXT:  %25 = StoreFrameInst %24: any, [x]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %28 = BinaryLeftShiftInst (:any) %26: any, %27: any
// CHECK-NEXT:  %29 = StoreFrameInst %28: any, [x]: any
// CHECK-NEXT:  %30 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %32 = BinaryRightShiftInst (:any) %30: any, %31: any
// CHECK-NEXT:  %33 = StoreFrameInst %32: any, [x]: any
// CHECK-NEXT:  %34 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %35 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %36 = BinaryUnsignedRightShiftInst (:any) %34: any, %35: any
// CHECK-NEXT:  %37 = StoreFrameInst %36: any, [x]: any
// CHECK-NEXT:  %38 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %39 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %40 = BinaryAndInst (:any) %38: any, %39: any
// CHECK-NEXT:  %41 = StoreFrameInst %40: any, [x]: any
// CHECK-NEXT:  %42 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %43 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %44 = BinaryXorInst (:any) %42: any, %43: any
// CHECK-NEXT:  %45 = StoreFrameInst %44: any, [x]: any
// CHECK-NEXT:  %46 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %47 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %48 = BinaryOrInst (:any) %46: any, %47: any
// CHECK-NEXT:  %49 = StoreFrameInst %48: any, [x]: any
// CHECK-NEXT:  %50 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %51 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %52 = StorePropertyLooseInst %51: any, %50: any, "t": string
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %54 = LoadPropertyInst (:any) %53: any, "t": string
// CHECK-NEXT:  %55 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %56 = BinaryAddInst (:any) %54: any, %55: any
// CHECK-NEXT:  %57 = StorePropertyLooseInst %56: any, %53: any, "t": string
// CHECK-NEXT:  %58 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %59 = LoadPropertyInst (:any) %58: any, "t": string
// CHECK-NEXT:  %60 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %61 = BinarySubtractInst (:any) %59: any, %60: any
// CHECK-NEXT:  %62 = StorePropertyLooseInst %61: any, %58: any, "t": string
// CHECK-NEXT:  %63 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %64 = LoadPropertyInst (:any) %63: any, "t": string
// CHECK-NEXT:  %65 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %66 = BinaryMultiplyInst (:any) %64: any, %65: any
// CHECK-NEXT:  %67 = StorePropertyLooseInst %66: any, %63: any, "t": string
// CHECK-NEXT:  %68 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %69 = LoadPropertyInst (:any) %68: any, "t": string
// CHECK-NEXT:  %70 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %71 = BinaryDivideInst (:any) %69: any, %70: any
// CHECK-NEXT:  %72 = StorePropertyLooseInst %71: any, %68: any, "t": string
// CHECK-NEXT:  %73 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %74 = LoadPropertyInst (:any) %73: any, "t": string
// CHECK-NEXT:  %75 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %76 = BinaryModuloInst (:any) %74: any, %75: any
// CHECK-NEXT:  %77 = StorePropertyLooseInst %76: any, %73: any, "t": string
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %79 = LoadPropertyInst (:any) %78: any, "t": string
// CHECK-NEXT:  %80 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %81 = BinaryLeftShiftInst (:any) %79: any, %80: any
// CHECK-NEXT:  %82 = StorePropertyLooseInst %81: any, %78: any, "t": string
// CHECK-NEXT:  %83 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %84 = LoadPropertyInst (:any) %83: any, "t": string
// CHECK-NEXT:  %85 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %86 = BinaryRightShiftInst (:any) %84: any, %85: any
// CHECK-NEXT:  %87 = StorePropertyLooseInst %86: any, %83: any, "t": string
// CHECK-NEXT:  %88 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %89 = LoadPropertyInst (:any) %88: any, "t": string
// CHECK-NEXT:  %90 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %91 = BinaryUnsignedRightShiftInst (:any) %89: any, %90: any
// CHECK-NEXT:  %92 = StorePropertyLooseInst %91: any, %88: any, "t": string
// CHECK-NEXT:  %93 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %94 = LoadPropertyInst (:any) %93: any, "t": string
// CHECK-NEXT:  %95 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %96 = BinaryAndInst (:any) %94: any, %95: any
// CHECK-NEXT:  %97 = StorePropertyLooseInst %96: any, %93: any, "t": string
// CHECK-NEXT:  %98 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %99 = LoadPropertyInst (:any) %98: any, "t": string
// CHECK-NEXT:  %100 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %101 = BinaryXorInst (:any) %99: any, %100: any
// CHECK-NEXT:  %102 = StorePropertyLooseInst %101: any, %98: any, "t": string
// CHECK-NEXT:  %103 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %104 = LoadPropertyInst (:any) %103: any, "t": string
// CHECK-NEXT:  %105 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %106 = BinaryOrInst (:any) %104: any, %105: any
// CHECK-NEXT:  %107 = StorePropertyLooseInst %106: any, %103: any, "t": string
// CHECK-NEXT:  %108 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %109 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %110 = BinaryEqualInst (:any) %108: any, %109: any
// CHECK-NEXT:  %111 = ReturnInst %110: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %112 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %113 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %114 = BinaryNotEqualInst (:any) %112: any, %113: any
// CHECK-NEXT:  %115 = ReturnInst %114: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %116 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %117 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %118 = BinaryStrictlyEqualInst (:any) %116: any, %117: any
// CHECK-NEXT:  %119 = ReturnInst %118: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %120 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %121 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %122 = BinaryLessThanInst (:any) %120: any, %121: any
// CHECK-NEXT:  %123 = ReturnInst %122: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %124 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %125 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %126 = BinaryLessThanOrEqualInst (:any) %124: any, %125: any
// CHECK-NEXT:  %127 = ReturnInst %126: any
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %128 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %129 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %130 = BinaryGreaterThanInst (:any) %128: any, %129: any
// CHECK-NEXT:  %131 = ReturnInst %130: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %132 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %133 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %134 = BinaryGreaterThanOrEqualInst (:any) %132: any, %133: any
// CHECK-NEXT:  %135 = ReturnInst %134: any
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %136 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %137 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %138 = BinaryLeftShiftInst (:any) %136: any, %137: any
// CHECK-NEXT:  %139 = ReturnInst %138: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %140 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %141 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %142 = BinaryLeftShiftInst (:any) %140: any, %141: any
// CHECK-NEXT:  %143 = ReturnInst %142: any
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %144 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %145 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %146 = BinaryUnsignedRightShiftInst (:any) %144: any, %145: any
// CHECK-NEXT:  %147 = ReturnInst %146: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %148 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %149 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %150 = BinaryAddInst (:any) %148: any, %149: any
// CHECK-NEXT:  %151 = ReturnInst %150: any
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %152 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %153 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %154 = BinarySubtractInst (:any) %152: any, %153: any
// CHECK-NEXT:  %155 = ReturnInst %154: any
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %156 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %157 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %158 = BinaryMultiplyInst (:any) %156: any, %157: any
// CHECK-NEXT:  %159 = ReturnInst %158: any
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %160 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %161 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %162 = BinaryDivideInst (:any) %160: any, %161: any
// CHECK-NEXT:  %163 = ReturnInst %162: any
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %164 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %165 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %166 = BinaryModuloInst (:any) %164: any, %165: any
// CHECK-NEXT:  %167 = ReturnInst %166: any
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %168 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %169 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %170 = BinaryOrInst (:any) %168: any, %169: any
// CHECK-NEXT:  %171 = ReturnInst %170: any
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %172 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %173 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %174 = BinaryXorInst (:any) %172: any, %173: any
// CHECK-NEXT:  %175 = ReturnInst %174: any
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %176 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %177 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %178 = BinaryAndInst (:any) %176: any, %177: any
// CHECK-NEXT:  %179 = ReturnInst %178: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %180 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %181 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %182 = BinaryInInst (:any) %180: any, %181: any
// CHECK-NEXT:  %183 = ReturnInst %182: any
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %184 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %185 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %186 = BinaryInstanceOfInst (:any) %184: any, %185: any
// CHECK-NEXT:  %187 = ReturnInst %186: any
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %188 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function assignment_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %6: any, %7: any
// CHECK-NEXT:  %9 = StoreFrameInst %8: any, [x]: any
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function member_test(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "t": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: any, %4: any, "t": string
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %6 = BinaryUnsignedRightShiftInst (:any) %4: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
