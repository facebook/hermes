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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo, assignment_test, member_test, binary_ops]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %assignment_test#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "assignment_test" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %member_test#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "member_test" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %binary_ops#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "binary_ops" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(x, y)#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x#2], %0
// CHECK-NEXT:  %5 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %6 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = StoreFrameInst %7, [x#2], %0
// CHECK-NEXT:  %9 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %10 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %11 = BinaryOperatorInst '-', %9, %10
// CHECK-NEXT:  %12 = StoreFrameInst %11, [x#2], %0
// CHECK-NEXT:  %13 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %14 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %15 = BinaryOperatorInst '*', %13, %14
// CHECK-NEXT:  %16 = StoreFrameInst %15, [x#2], %0
// CHECK-NEXT:  %17 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %18 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %19 = BinaryOperatorInst '/', %17, %18
// CHECK-NEXT:  %20 = StoreFrameInst %19, [x#2], %0
// CHECK-NEXT:  %21 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %22 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %23 = BinaryOperatorInst '%', %21, %22
// CHECK-NEXT:  %24 = StoreFrameInst %23, [x#2], %0
// CHECK-NEXT:  %25 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %26 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %27 = BinaryOperatorInst '<<', %25, %26
// CHECK-NEXT:  %28 = StoreFrameInst %27, [x#2], %0
// CHECK-NEXT:  %29 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %30 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %31 = BinaryOperatorInst '>>', %29, %30
// CHECK-NEXT:  %32 = StoreFrameInst %31, [x#2], %0
// CHECK-NEXT:  %33 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %34 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %35 = BinaryOperatorInst '>>>', %33, %34
// CHECK-NEXT:  %36 = StoreFrameInst %35, [x#2], %0
// CHECK-NEXT:  %37 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %38 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %39 = BinaryOperatorInst '&', %37, %38
// CHECK-NEXT:  %40 = StoreFrameInst %39, [x#2], %0
// CHECK-NEXT:  %41 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %42 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %43 = BinaryOperatorInst '^', %41, %42
// CHECK-NEXT:  %44 = StoreFrameInst %43, [x#2], %0
// CHECK-NEXT:  %45 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %46 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %47 = BinaryOperatorInst '|', %45, %46
// CHECK-NEXT:  %48 = StoreFrameInst %47, [x#2], %0
// CHECK-NEXT:  %49 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %50 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %51 = StorePropertyInst %50, %49, "t" : string
// CHECK-NEXT:  %52 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %53 = LoadPropertyInst %52, "t" : string
// CHECK-NEXT:  %54 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %55 = BinaryOperatorInst '+', %53, %54
// CHECK-NEXT:  %56 = StorePropertyInst %55, %52, "t" : string
// CHECK-NEXT:  %57 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %58 = LoadPropertyInst %57, "t" : string
// CHECK-NEXT:  %59 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %60 = BinaryOperatorInst '-', %58, %59
// CHECK-NEXT:  %61 = StorePropertyInst %60, %57, "t" : string
// CHECK-NEXT:  %62 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %63 = LoadPropertyInst %62, "t" : string
// CHECK-NEXT:  %64 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %65 = BinaryOperatorInst '*', %63, %64
// CHECK-NEXT:  %66 = StorePropertyInst %65, %62, "t" : string
// CHECK-NEXT:  %67 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %68 = LoadPropertyInst %67, "t" : string
// CHECK-NEXT:  %69 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %70 = BinaryOperatorInst '/', %68, %69
// CHECK-NEXT:  %71 = StorePropertyInst %70, %67, "t" : string
// CHECK-NEXT:  %72 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %73 = LoadPropertyInst %72, "t" : string
// CHECK-NEXT:  %74 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %75 = BinaryOperatorInst '%', %73, %74
// CHECK-NEXT:  %76 = StorePropertyInst %75, %72, "t" : string
// CHECK-NEXT:  %77 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %78 = LoadPropertyInst %77, "t" : string
// CHECK-NEXT:  %79 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %80 = BinaryOperatorInst '<<', %78, %79
// CHECK-NEXT:  %81 = StorePropertyInst %80, %77, "t" : string
// CHECK-NEXT:  %82 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %83 = LoadPropertyInst %82, "t" : string
// CHECK-NEXT:  %84 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %85 = BinaryOperatorInst '>>', %83, %84
// CHECK-NEXT:  %86 = StorePropertyInst %85, %82, "t" : string
// CHECK-NEXT:  %87 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %88 = LoadPropertyInst %87, "t" : string
// CHECK-NEXT:  %89 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %90 = BinaryOperatorInst '>>>', %88, %89
// CHECK-NEXT:  %91 = StorePropertyInst %90, %87, "t" : string
// CHECK-NEXT:  %92 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %93 = LoadPropertyInst %92, "t" : string
// CHECK-NEXT:  %94 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %95 = BinaryOperatorInst '&', %93, %94
// CHECK-NEXT:  %96 = StorePropertyInst %95, %92, "t" : string
// CHECK-NEXT:  %97 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %98 = LoadPropertyInst %97, "t" : string
// CHECK-NEXT:  %99 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %100 = BinaryOperatorInst '^', %98, %99
// CHECK-NEXT:  %101 = StorePropertyInst %100, %97, "t" : string
// CHECK-NEXT:  %102 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %103 = LoadPropertyInst %102, "t" : string
// CHECK-NEXT:  %104 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %105 = BinaryOperatorInst '|', %103, %104
// CHECK-NEXT:  %106 = StorePropertyInst %105, %102, "t" : string
// CHECK-NEXT:  %107 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %108 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %109 = BinaryOperatorInst '==', %107, %108
// CHECK-NEXT:  %110 = ReturnInst %109
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %111 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %112 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %113 = BinaryOperatorInst '!=', %111, %112
// CHECK-NEXT:  %114 = ReturnInst %113
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %115 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %116 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %117 = BinaryOperatorInst '===', %115, %116
// CHECK-NEXT:  %118 = ReturnInst %117
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %119 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %120 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %121 = BinaryOperatorInst '<', %119, %120
// CHECK-NEXT:  %122 = ReturnInst %121
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %123 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %124 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %125 = BinaryOperatorInst '<=', %123, %124
// CHECK-NEXT:  %126 = ReturnInst %125
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %127 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %128 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %129 = BinaryOperatorInst '>', %127, %128
// CHECK-NEXT:  %130 = ReturnInst %129
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %131 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %132 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %133 = BinaryOperatorInst '>=', %131, %132
// CHECK-NEXT:  %134 = ReturnInst %133
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %135 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %136 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %137 = BinaryOperatorInst '<<', %135, %136
// CHECK-NEXT:  %138 = ReturnInst %137
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %139 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %140 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %141 = BinaryOperatorInst '<<', %139, %140
// CHECK-NEXT:  %142 = ReturnInst %141
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %143 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %144 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %145 = BinaryOperatorInst '>>>', %143, %144
// CHECK-NEXT:  %146 = ReturnInst %145
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %147 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %148 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %149 = BinaryOperatorInst '+', %147, %148
// CHECK-NEXT:  %150 = ReturnInst %149
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %151 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %152 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %153 = BinaryOperatorInst '-', %151, %152
// CHECK-NEXT:  %154 = ReturnInst %153
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %155 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %156 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %157 = BinaryOperatorInst '*', %155, %156
// CHECK-NEXT:  %158 = ReturnInst %157
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %159 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %160 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %161 = BinaryOperatorInst '/', %159, %160
// CHECK-NEXT:  %162 = ReturnInst %161
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %163 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %164 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %165 = BinaryOperatorInst '%', %163, %164
// CHECK-NEXT:  %166 = ReturnInst %165
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %167 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %168 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %169 = BinaryOperatorInst '|', %167, %168
// CHECK-NEXT:  %170 = ReturnInst %169
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %171 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %172 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %173 = BinaryOperatorInst '^', %171, %172
// CHECK-NEXT:  %174 = ReturnInst %173
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %175 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %176 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %177 = BinaryOperatorInst '&', %175, %176
// CHECK-NEXT:  %178 = ReturnInst %177
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %179 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %180 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %181 = BinaryOperatorInst 'in', %179, %180
// CHECK-NEXT:  %182 = ReturnInst %181
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %183 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %184 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %185 = BinaryOperatorInst 'instanceof', %183, %184
// CHECK-NEXT:  %186 = ReturnInst %185
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %187 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function assignment_test#0#1(x, y)#3
// CHECK-NEXT:frame = [x#3, y#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{assignment_test#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#3], %0
// CHECK-NEXT:  %3 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst %3, [x#3], %0
// CHECK-NEXT:  %5 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %6 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = StoreFrameInst %7, [x#3], %0
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function member_test#0#1(x, y)#4
// CHECK-NEXT:frame = [x#4, y#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{member_test#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#4], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#4], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "t" : string
// CHECK-NEXT:  %5 = LoadFrameInst [y#4], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = StorePropertyInst %6, %3, "t" : string
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops#0#1(x, y)#5
// CHECK-NEXT:frame = [x#5, y#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{binary_ops#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#5], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#5], %0
// CHECK-NEXT:  %4 = LoadFrameInst [y#5], %0
// CHECK-NEXT:  %5 = BinaryOperatorInst '>>>', %3, %4
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
