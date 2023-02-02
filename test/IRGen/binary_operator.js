/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "assignment_test" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "member_test" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "binary_ops" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %foo()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %assignment_test()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "assignment_test" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %member_test()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "member_test" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %binary_ops()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "binary_ops" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
// CHECK-NEXT:function_end

// CHECK:function foo(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x]
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = LoadFrameInst [y]
// CHECK-NEXT:  %8 = BinaryOperatorInst '+', %6, %7
// CHECK-NEXT:  %9 = StoreFrameInst %8, [x]
// CHECK-NEXT:  %10 = LoadFrameInst [x]
// CHECK-NEXT:  %11 = LoadFrameInst [y]
// CHECK-NEXT:  %12 = BinaryOperatorInst '-', %10, %11
// CHECK-NEXT:  %13 = StoreFrameInst %12, [x]
// CHECK-NEXT:  %14 = LoadFrameInst [x]
// CHECK-NEXT:  %15 = LoadFrameInst [y]
// CHECK-NEXT:  %16 = BinaryOperatorInst '*', %14, %15
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = LoadFrameInst [x]
// CHECK-NEXT:  %19 = LoadFrameInst [y]
// CHECK-NEXT:  %20 = BinaryOperatorInst '/', %18, %19
// CHECK-NEXT:  %21 = StoreFrameInst %20, [x]
// CHECK-NEXT:  %22 = LoadFrameInst [x]
// CHECK-NEXT:  %23 = LoadFrameInst [y]
// CHECK-NEXT:  %24 = BinaryOperatorInst '%', %22, %23
// CHECK-NEXT:  %25 = StoreFrameInst %24, [x]
// CHECK-NEXT:  %26 = LoadFrameInst [x]
// CHECK-NEXT:  %27 = LoadFrameInst [y]
// CHECK-NEXT:  %28 = BinaryOperatorInst '<<', %26, %27
// CHECK-NEXT:  %29 = StoreFrameInst %28, [x]
// CHECK-NEXT:  %30 = LoadFrameInst [x]
// CHECK-NEXT:  %31 = LoadFrameInst [y]
// CHECK-NEXT:  %32 = BinaryOperatorInst '>>', %30, %31
// CHECK-NEXT:  %33 = StoreFrameInst %32, [x]
// CHECK-NEXT:  %34 = LoadFrameInst [x]
// CHECK-NEXT:  %35 = LoadFrameInst [y]
// CHECK-NEXT:  %36 = BinaryOperatorInst '>>>', %34, %35
// CHECK-NEXT:  %37 = StoreFrameInst %36, [x]
// CHECK-NEXT:  %38 = LoadFrameInst [x]
// CHECK-NEXT:  %39 = LoadFrameInst [y]
// CHECK-NEXT:  %40 = BinaryOperatorInst '&', %38, %39
// CHECK-NEXT:  %41 = StoreFrameInst %40, [x]
// CHECK-NEXT:  %42 = LoadFrameInst [x]
// CHECK-NEXT:  %43 = LoadFrameInst [y]
// CHECK-NEXT:  %44 = BinaryOperatorInst '^', %42, %43
// CHECK-NEXT:  %45 = StoreFrameInst %44, [x]
// CHECK-NEXT:  %46 = LoadFrameInst [x]
// CHECK-NEXT:  %47 = LoadFrameInst [y]
// CHECK-NEXT:  %48 = BinaryOperatorInst '|', %46, %47
// CHECK-NEXT:  %49 = StoreFrameInst %48, [x]
// CHECK-NEXT:  %50 = LoadFrameInst [x]
// CHECK-NEXT:  %51 = LoadFrameInst [y]
// CHECK-NEXT:  %52 = StorePropertyLooseInst %51, %50, "t" : string
// CHECK-NEXT:  %53 = LoadFrameInst [x]
// CHECK-NEXT:  %54 = LoadPropertyInst %53, "t" : string
// CHECK-NEXT:  %55 = LoadFrameInst [y]
// CHECK-NEXT:  %56 = BinaryOperatorInst '+', %54, %55
// CHECK-NEXT:  %57 = StorePropertyLooseInst %56, %53, "t" : string
// CHECK-NEXT:  %58 = LoadFrameInst [x]
// CHECK-NEXT:  %59 = LoadPropertyInst %58, "t" : string
// CHECK-NEXT:  %60 = LoadFrameInst [y]
// CHECK-NEXT:  %61 = BinaryOperatorInst '-', %59, %60
// CHECK-NEXT:  %62 = StorePropertyLooseInst %61, %58, "t" : string
// CHECK-NEXT:  %63 = LoadFrameInst [x]
// CHECK-NEXT:  %64 = LoadPropertyInst %63, "t" : string
// CHECK-NEXT:  %65 = LoadFrameInst [y]
// CHECK-NEXT:  %66 = BinaryOperatorInst '*', %64, %65
// CHECK-NEXT:  %67 = StorePropertyLooseInst %66, %63, "t" : string
// CHECK-NEXT:  %68 = LoadFrameInst [x]
// CHECK-NEXT:  %69 = LoadPropertyInst %68, "t" : string
// CHECK-NEXT:  %70 = LoadFrameInst [y]
// CHECK-NEXT:  %71 = BinaryOperatorInst '/', %69, %70
// CHECK-NEXT:  %72 = StorePropertyLooseInst %71, %68, "t" : string
// CHECK-NEXT:  %73 = LoadFrameInst [x]
// CHECK-NEXT:  %74 = LoadPropertyInst %73, "t" : string
// CHECK-NEXT:  %75 = LoadFrameInst [y]
// CHECK-NEXT:  %76 = BinaryOperatorInst '%', %74, %75
// CHECK-NEXT:  %77 = StorePropertyLooseInst %76, %73, "t" : string
// CHECK-NEXT:  %78 = LoadFrameInst [x]
// CHECK-NEXT:  %79 = LoadPropertyInst %78, "t" : string
// CHECK-NEXT:  %80 = LoadFrameInst [y]
// CHECK-NEXT:  %81 = BinaryOperatorInst '<<', %79, %80
// CHECK-NEXT:  %82 = StorePropertyLooseInst %81, %78, "t" : string
// CHECK-NEXT:  %83 = LoadFrameInst [x]
// CHECK-NEXT:  %84 = LoadPropertyInst %83, "t" : string
// CHECK-NEXT:  %85 = LoadFrameInst [y]
// CHECK-NEXT:  %86 = BinaryOperatorInst '>>', %84, %85
// CHECK-NEXT:  %87 = StorePropertyLooseInst %86, %83, "t" : string
// CHECK-NEXT:  %88 = LoadFrameInst [x]
// CHECK-NEXT:  %89 = LoadPropertyInst %88, "t" : string
// CHECK-NEXT:  %90 = LoadFrameInst [y]
// CHECK-NEXT:  %91 = BinaryOperatorInst '>>>', %89, %90
// CHECK-NEXT:  %92 = StorePropertyLooseInst %91, %88, "t" : string
// CHECK-NEXT:  %93 = LoadFrameInst [x]
// CHECK-NEXT:  %94 = LoadPropertyInst %93, "t" : string
// CHECK-NEXT:  %95 = LoadFrameInst [y]
// CHECK-NEXT:  %96 = BinaryOperatorInst '&', %94, %95
// CHECK-NEXT:  %97 = StorePropertyLooseInst %96, %93, "t" : string
// CHECK-NEXT:  %98 = LoadFrameInst [x]
// CHECK-NEXT:  %99 = LoadPropertyInst %98, "t" : string
// CHECK-NEXT:  %100 = LoadFrameInst [y]
// CHECK-NEXT:  %101 = BinaryOperatorInst '^', %99, %100
// CHECK-NEXT:  %102 = StorePropertyLooseInst %101, %98, "t" : string
// CHECK-NEXT:  %103 = LoadFrameInst [x]
// CHECK-NEXT:  %104 = LoadPropertyInst %103, "t" : string
// CHECK-NEXT:  %105 = LoadFrameInst [y]
// CHECK-NEXT:  %106 = BinaryOperatorInst '|', %104, %105
// CHECK-NEXT:  %107 = StorePropertyLooseInst %106, %103, "t" : string
// CHECK-NEXT:  %108 = LoadFrameInst [x]
// CHECK-NEXT:  %109 = LoadFrameInst [y]
// CHECK-NEXT:  %110 = BinaryOperatorInst '==', %108, %109
// CHECK-NEXT:  %111 = ReturnInst %110
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %112 = LoadFrameInst [x]
// CHECK-NEXT:  %113 = LoadFrameInst [y]
// CHECK-NEXT:  %114 = BinaryOperatorInst '!=', %112, %113
// CHECK-NEXT:  %115 = ReturnInst %114
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %116 = LoadFrameInst [x]
// CHECK-NEXT:  %117 = LoadFrameInst [y]
// CHECK-NEXT:  %118 = BinaryOperatorInst '===', %116, %117
// CHECK-NEXT:  %119 = ReturnInst %118
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %120 = LoadFrameInst [x]
// CHECK-NEXT:  %121 = LoadFrameInst [y]
// CHECK-NEXT:  %122 = BinaryOperatorInst '<', %120, %121
// CHECK-NEXT:  %123 = ReturnInst %122
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %124 = LoadFrameInst [x]
// CHECK-NEXT:  %125 = LoadFrameInst [y]
// CHECK-NEXT:  %126 = BinaryOperatorInst '<=', %124, %125
// CHECK-NEXT:  %127 = ReturnInst %126
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %128 = LoadFrameInst [x]
// CHECK-NEXT:  %129 = LoadFrameInst [y]
// CHECK-NEXT:  %130 = BinaryOperatorInst '>', %128, %129
// CHECK-NEXT:  %131 = ReturnInst %130
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %132 = LoadFrameInst [x]
// CHECK-NEXT:  %133 = LoadFrameInst [y]
// CHECK-NEXT:  %134 = BinaryOperatorInst '>=', %132, %133
// CHECK-NEXT:  %135 = ReturnInst %134
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %136 = LoadFrameInst [x]
// CHECK-NEXT:  %137 = LoadFrameInst [y]
// CHECK-NEXT:  %138 = BinaryOperatorInst '<<', %136, %137
// CHECK-NEXT:  %139 = ReturnInst %138
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %140 = LoadFrameInst [x]
// CHECK-NEXT:  %141 = LoadFrameInst [y]
// CHECK-NEXT:  %142 = BinaryOperatorInst '<<', %140, %141
// CHECK-NEXT:  %143 = ReturnInst %142
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %144 = LoadFrameInst [x]
// CHECK-NEXT:  %145 = LoadFrameInst [y]
// CHECK-NEXT:  %146 = BinaryOperatorInst '>>>', %144, %145
// CHECK-NEXT:  %147 = ReturnInst %146
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %148 = LoadFrameInst [x]
// CHECK-NEXT:  %149 = LoadFrameInst [y]
// CHECK-NEXT:  %150 = BinaryOperatorInst '+', %148, %149
// CHECK-NEXT:  %151 = ReturnInst %150
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %152 = LoadFrameInst [x]
// CHECK-NEXT:  %153 = LoadFrameInst [y]
// CHECK-NEXT:  %154 = BinaryOperatorInst '-', %152, %153
// CHECK-NEXT:  %155 = ReturnInst %154
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %156 = LoadFrameInst [x]
// CHECK-NEXT:  %157 = LoadFrameInst [y]
// CHECK-NEXT:  %158 = BinaryOperatorInst '*', %156, %157
// CHECK-NEXT:  %159 = ReturnInst %158
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %160 = LoadFrameInst [x]
// CHECK-NEXT:  %161 = LoadFrameInst [y]
// CHECK-NEXT:  %162 = BinaryOperatorInst '/', %160, %161
// CHECK-NEXT:  %163 = ReturnInst %162
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %164 = LoadFrameInst [x]
// CHECK-NEXT:  %165 = LoadFrameInst [y]
// CHECK-NEXT:  %166 = BinaryOperatorInst '%', %164, %165
// CHECK-NEXT:  %167 = ReturnInst %166
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %168 = LoadFrameInst [x]
// CHECK-NEXT:  %169 = LoadFrameInst [y]
// CHECK-NEXT:  %170 = BinaryOperatorInst '|', %168, %169
// CHECK-NEXT:  %171 = ReturnInst %170
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %172 = LoadFrameInst [x]
// CHECK-NEXT:  %173 = LoadFrameInst [y]
// CHECK-NEXT:  %174 = BinaryOperatorInst '^', %172, %173
// CHECK-NEXT:  %175 = ReturnInst %174
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %176 = LoadFrameInst [x]
// CHECK-NEXT:  %177 = LoadFrameInst [y]
// CHECK-NEXT:  %178 = BinaryOperatorInst '&', %176, %177
// CHECK-NEXT:  %179 = ReturnInst %178
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %180 = LoadFrameInst [x]
// CHECK-NEXT:  %181 = LoadFrameInst [y]
// CHECK-NEXT:  %182 = BinaryOperatorInst 'in', %180, %181
// CHECK-NEXT:  %183 = ReturnInst %182
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %184 = LoadFrameInst [x]
// CHECK-NEXT:  %185 = LoadFrameInst [y]
// CHECK-NEXT:  %186 = BinaryOperatorInst 'instanceof', %184, %185
// CHECK-NEXT:  %187 = ReturnInst %186
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %188 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function assignment_test(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = StoreFrameInst %4, [x]
// CHECK-NEXT:  %6 = LoadFrameInst [x]
// CHECK-NEXT:  %7 = LoadFrameInst [y]
// CHECK-NEXT:  %8 = BinaryOperatorInst '+', %6, %7
// CHECK-NEXT:  %9 = StoreFrameInst %8, [x]
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function member_test(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "t" : string
// CHECK-NEXT:  %6 = LoadFrameInst [y]
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7, %4, "t" : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = BinaryOperatorInst '>>>', %4, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
