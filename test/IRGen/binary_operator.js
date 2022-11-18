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
// CHECK-NEXT:frame = [], globals = [foo, assignment_test, member_test, binary_ops]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %assignment_test()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "assignment_test" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %member_test()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "member_test" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %binary_ops()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "binary_ops" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function foo(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [y]
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = StoreFrameInst %6, [x]
// CHECK-NEXT:  %8 = LoadFrameInst [x]
// CHECK-NEXT:  %9 = LoadFrameInst [y]
// CHECK-NEXT:  %10 = BinaryOperatorInst '-', %8, %9
// CHECK-NEXT:  %11 = StoreFrameInst %10, [x]
// CHECK-NEXT:  %12 = LoadFrameInst [x]
// CHECK-NEXT:  %13 = LoadFrameInst [y]
// CHECK-NEXT:  %14 = BinaryOperatorInst '*', %12, %13
// CHECK-NEXT:  %15 = StoreFrameInst %14, [x]
// CHECK-NEXT:  %16 = LoadFrameInst [x]
// CHECK-NEXT:  %17 = LoadFrameInst [y]
// CHECK-NEXT:  %18 = BinaryOperatorInst '/', %16, %17
// CHECK-NEXT:  %19 = StoreFrameInst %18, [x]
// CHECK-NEXT:  %20 = LoadFrameInst [x]
// CHECK-NEXT:  %21 = LoadFrameInst [y]
// CHECK-NEXT:  %22 = BinaryOperatorInst '%', %20, %21
// CHECK-NEXT:  %23 = StoreFrameInst %22, [x]
// CHECK-NEXT:  %24 = LoadFrameInst [x]
// CHECK-NEXT:  %25 = LoadFrameInst [y]
// CHECK-NEXT:  %26 = BinaryOperatorInst '<<', %24, %25
// CHECK-NEXT:  %27 = StoreFrameInst %26, [x]
// CHECK-NEXT:  %28 = LoadFrameInst [x]
// CHECK-NEXT:  %29 = LoadFrameInst [y]
// CHECK-NEXT:  %30 = BinaryOperatorInst '>>', %28, %29
// CHECK-NEXT:  %31 = StoreFrameInst %30, [x]
// CHECK-NEXT:  %32 = LoadFrameInst [x]
// CHECK-NEXT:  %33 = LoadFrameInst [y]
// CHECK-NEXT:  %34 = BinaryOperatorInst '>>>', %32, %33
// CHECK-NEXT:  %35 = StoreFrameInst %34, [x]
// CHECK-NEXT:  %36 = LoadFrameInst [x]
// CHECK-NEXT:  %37 = LoadFrameInst [y]
// CHECK-NEXT:  %38 = BinaryOperatorInst '&', %36, %37
// CHECK-NEXT:  %39 = StoreFrameInst %38, [x]
// CHECK-NEXT:  %40 = LoadFrameInst [x]
// CHECK-NEXT:  %41 = LoadFrameInst [y]
// CHECK-NEXT:  %42 = BinaryOperatorInst '^', %40, %41
// CHECK-NEXT:  %43 = StoreFrameInst %42, [x]
// CHECK-NEXT:  %44 = LoadFrameInst [x]
// CHECK-NEXT:  %45 = LoadFrameInst [y]
// CHECK-NEXT:  %46 = BinaryOperatorInst '|', %44, %45
// CHECK-NEXT:  %47 = StoreFrameInst %46, [x]
// CHECK-NEXT:  %48 = LoadFrameInst [x]
// CHECK-NEXT:  %49 = LoadFrameInst [y]
// CHECK-NEXT:  %50 = StorePropertyLooseInst %49, %48, "t" : string
// CHECK-NEXT:  %51 = LoadFrameInst [x]
// CHECK-NEXT:  %52 = LoadPropertyInst %51, "t" : string
// CHECK-NEXT:  %53 = LoadFrameInst [y]
// CHECK-NEXT:  %54 = BinaryOperatorInst '+', %52, %53
// CHECK-NEXT:  %55 = StorePropertyLooseInst %54, %51, "t" : string
// CHECK-NEXT:  %56 = LoadFrameInst [x]
// CHECK-NEXT:  %57 = LoadPropertyInst %56, "t" : string
// CHECK-NEXT:  %58 = LoadFrameInst [y]
// CHECK-NEXT:  %59 = BinaryOperatorInst '-', %57, %58
// CHECK-NEXT:  %60 = StorePropertyLooseInst %59, %56, "t" : string
// CHECK-NEXT:  %61 = LoadFrameInst [x]
// CHECK-NEXT:  %62 = LoadPropertyInst %61, "t" : string
// CHECK-NEXT:  %63 = LoadFrameInst [y]
// CHECK-NEXT:  %64 = BinaryOperatorInst '*', %62, %63
// CHECK-NEXT:  %65 = StorePropertyLooseInst %64, %61, "t" : string
// CHECK-NEXT:  %66 = LoadFrameInst [x]
// CHECK-NEXT:  %67 = LoadPropertyInst %66, "t" : string
// CHECK-NEXT:  %68 = LoadFrameInst [y]
// CHECK-NEXT:  %69 = BinaryOperatorInst '/', %67, %68
// CHECK-NEXT:  %70 = StorePropertyLooseInst %69, %66, "t" : string
// CHECK-NEXT:  %71 = LoadFrameInst [x]
// CHECK-NEXT:  %72 = LoadPropertyInst %71, "t" : string
// CHECK-NEXT:  %73 = LoadFrameInst [y]
// CHECK-NEXT:  %74 = BinaryOperatorInst '%', %72, %73
// CHECK-NEXT:  %75 = StorePropertyLooseInst %74, %71, "t" : string
// CHECK-NEXT:  %76 = LoadFrameInst [x]
// CHECK-NEXT:  %77 = LoadPropertyInst %76, "t" : string
// CHECK-NEXT:  %78 = LoadFrameInst [y]
// CHECK-NEXT:  %79 = BinaryOperatorInst '<<', %77, %78
// CHECK-NEXT:  %80 = StorePropertyLooseInst %79, %76, "t" : string
// CHECK-NEXT:  %81 = LoadFrameInst [x]
// CHECK-NEXT:  %82 = LoadPropertyInst %81, "t" : string
// CHECK-NEXT:  %83 = LoadFrameInst [y]
// CHECK-NEXT:  %84 = BinaryOperatorInst '>>', %82, %83
// CHECK-NEXT:  %85 = StorePropertyLooseInst %84, %81, "t" : string
// CHECK-NEXT:  %86 = LoadFrameInst [x]
// CHECK-NEXT:  %87 = LoadPropertyInst %86, "t" : string
// CHECK-NEXT:  %88 = LoadFrameInst [y]
// CHECK-NEXT:  %89 = BinaryOperatorInst '>>>', %87, %88
// CHECK-NEXT:  %90 = StorePropertyLooseInst %89, %86, "t" : string
// CHECK-NEXT:  %91 = LoadFrameInst [x]
// CHECK-NEXT:  %92 = LoadPropertyInst %91, "t" : string
// CHECK-NEXT:  %93 = LoadFrameInst [y]
// CHECK-NEXT:  %94 = BinaryOperatorInst '&', %92, %93
// CHECK-NEXT:  %95 = StorePropertyLooseInst %94, %91, "t" : string
// CHECK-NEXT:  %96 = LoadFrameInst [x]
// CHECK-NEXT:  %97 = LoadPropertyInst %96, "t" : string
// CHECK-NEXT:  %98 = LoadFrameInst [y]
// CHECK-NEXT:  %99 = BinaryOperatorInst '^', %97, %98
// CHECK-NEXT:  %100 = StorePropertyLooseInst %99, %96, "t" : string
// CHECK-NEXT:  %101 = LoadFrameInst [x]
// CHECK-NEXT:  %102 = LoadPropertyInst %101, "t" : string
// CHECK-NEXT:  %103 = LoadFrameInst [y]
// CHECK-NEXT:  %104 = BinaryOperatorInst '|', %102, %103
// CHECK-NEXT:  %105 = StorePropertyLooseInst %104, %101, "t" : string
// CHECK-NEXT:  %106 = LoadFrameInst [x]
// CHECK-NEXT:  %107 = LoadFrameInst [y]
// CHECK-NEXT:  %108 = BinaryOperatorInst '==', %106, %107
// CHECK-NEXT:  %109 = ReturnInst %108
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %110 = LoadFrameInst [x]
// CHECK-NEXT:  %111 = LoadFrameInst [y]
// CHECK-NEXT:  %112 = BinaryOperatorInst '!=', %110, %111
// CHECK-NEXT:  %113 = ReturnInst %112
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %114 = LoadFrameInst [x]
// CHECK-NEXT:  %115 = LoadFrameInst [y]
// CHECK-NEXT:  %116 = BinaryOperatorInst '===', %114, %115
// CHECK-NEXT:  %117 = ReturnInst %116
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %118 = LoadFrameInst [x]
// CHECK-NEXT:  %119 = LoadFrameInst [y]
// CHECK-NEXT:  %120 = BinaryOperatorInst '<', %118, %119
// CHECK-NEXT:  %121 = ReturnInst %120
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %122 = LoadFrameInst [x]
// CHECK-NEXT:  %123 = LoadFrameInst [y]
// CHECK-NEXT:  %124 = BinaryOperatorInst '<=', %122, %123
// CHECK-NEXT:  %125 = ReturnInst %124
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %126 = LoadFrameInst [x]
// CHECK-NEXT:  %127 = LoadFrameInst [y]
// CHECK-NEXT:  %128 = BinaryOperatorInst '>', %126, %127
// CHECK-NEXT:  %129 = ReturnInst %128
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %130 = LoadFrameInst [x]
// CHECK-NEXT:  %131 = LoadFrameInst [y]
// CHECK-NEXT:  %132 = BinaryOperatorInst '>=', %130, %131
// CHECK-NEXT:  %133 = ReturnInst %132
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %134 = LoadFrameInst [x]
// CHECK-NEXT:  %135 = LoadFrameInst [y]
// CHECK-NEXT:  %136 = BinaryOperatorInst '<<', %134, %135
// CHECK-NEXT:  %137 = ReturnInst %136
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %138 = LoadFrameInst [x]
// CHECK-NEXT:  %139 = LoadFrameInst [y]
// CHECK-NEXT:  %140 = BinaryOperatorInst '<<', %138, %139
// CHECK-NEXT:  %141 = ReturnInst %140
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %142 = LoadFrameInst [x]
// CHECK-NEXT:  %143 = LoadFrameInst [y]
// CHECK-NEXT:  %144 = BinaryOperatorInst '>>>', %142, %143
// CHECK-NEXT:  %145 = ReturnInst %144
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %146 = LoadFrameInst [x]
// CHECK-NEXT:  %147 = LoadFrameInst [y]
// CHECK-NEXT:  %148 = BinaryOperatorInst '+', %146, %147
// CHECK-NEXT:  %149 = ReturnInst %148
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %150 = LoadFrameInst [x]
// CHECK-NEXT:  %151 = LoadFrameInst [y]
// CHECK-NEXT:  %152 = BinaryOperatorInst '-', %150, %151
// CHECK-NEXT:  %153 = ReturnInst %152
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %154 = LoadFrameInst [x]
// CHECK-NEXT:  %155 = LoadFrameInst [y]
// CHECK-NEXT:  %156 = BinaryOperatorInst '*', %154, %155
// CHECK-NEXT:  %157 = ReturnInst %156
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %158 = LoadFrameInst [x]
// CHECK-NEXT:  %159 = LoadFrameInst [y]
// CHECK-NEXT:  %160 = BinaryOperatorInst '/', %158, %159
// CHECK-NEXT:  %161 = ReturnInst %160
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %162 = LoadFrameInst [x]
// CHECK-NEXT:  %163 = LoadFrameInst [y]
// CHECK-NEXT:  %164 = BinaryOperatorInst '%', %162, %163
// CHECK-NEXT:  %165 = ReturnInst %164
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %166 = LoadFrameInst [x]
// CHECK-NEXT:  %167 = LoadFrameInst [y]
// CHECK-NEXT:  %168 = BinaryOperatorInst '|', %166, %167
// CHECK-NEXT:  %169 = ReturnInst %168
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %170 = LoadFrameInst [x]
// CHECK-NEXT:  %171 = LoadFrameInst [y]
// CHECK-NEXT:  %172 = BinaryOperatorInst '^', %170, %171
// CHECK-NEXT:  %173 = ReturnInst %172
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %174 = LoadFrameInst [x]
// CHECK-NEXT:  %175 = LoadFrameInst [y]
// CHECK-NEXT:  %176 = BinaryOperatorInst '&', %174, %175
// CHECK-NEXT:  %177 = ReturnInst %176
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %178 = LoadFrameInst [x]
// CHECK-NEXT:  %179 = LoadFrameInst [y]
// CHECK-NEXT:  %180 = BinaryOperatorInst 'in', %178, %179
// CHECK-NEXT:  %181 = ReturnInst %180
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %182 = LoadFrameInst [x]
// CHECK-NEXT:  %183 = LoadFrameInst [y]
// CHECK-NEXT:  %184 = BinaryOperatorInst 'instanceof', %182, %183
// CHECK-NEXT:  %185 = ReturnInst %184
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %186 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function assignment_test(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [y]
// CHECK-NEXT:  %3 = StoreFrameInst %2, [x]
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
// CHECK-NEXT:  %7 = StoreFrameInst %6, [x]
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function member_test(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "t" : string
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3, %4
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5, %2, "t" : string
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function binary_ops(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = LoadFrameInst [y]
// CHECK-NEXT:  %4 = BinaryOperatorInst '>>>', %2, %3
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
