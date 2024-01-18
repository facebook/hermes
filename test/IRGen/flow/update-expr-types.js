/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=f1,f2,f3,f4,f5,f6 -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that the type of the lvalue load is correct and the type of the whole
// expression is correct.

function f1(n: number): number {
    return ++n;
}
function f2(n: number): number {
    return n++;
}
function f3(n: any): any {
    return ++n;
}
function f4(n: any): any {
    return n++;
}
function f5(n: number): number {
    return n += 10;
}
function f6(n: number, a: any): number {
    return n += a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function f1(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %0: number, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:  %4 = UnaryIncInst (:number) %3: number
// CHECK-NEXT:       StoreFrameInst %4: number, [n]: any
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:function f2(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %0: number, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:  %4 = UnaryIncInst (:number) %3: number
// CHECK-NEXT:       StoreFrameInst %4: number, [n]: any
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function f3(n: any): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %0: any, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = UnaryIncInst (:number|bigint) %2: any
// CHECK-NEXT:       StoreFrameInst %3: number|bigint, [n]: any
// CHECK-NEXT:       ReturnInst %3: number|bigint
// CHECK-NEXT:function_end

// CHECK:function f4(n: any): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %n: any
// CHECK-NEXT:       StoreFrameInst %0: any, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = AsNumericInst (:number|bigint) %2: any
// CHECK-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: number|bigint
// CHECK-NEXT:       StoreFrameInst %4: number|bigint, [n]: any
// CHECK-NEXT:       ReturnInst %3: number|bigint
// CHECK-NEXT:function_end

// CHECK:function f5(n: number): any [typed]
// CHECK-NEXT:frame = [n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %0: number, [n]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:  %4 = BinaryAddInst (:any) %3: number, 10: number
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       StoreFrameInst %5: number, [n]: any
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function f6(n: number, a: any): any [typed]
// CHECK-NEXT:frame = [n: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %0: number, [n]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: number, %6: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:number) %7: any, type(number)
// CHECK-NEXT:       StoreFrameInst %8: number, [n]: any
// CHECK-NEXT:        ReturnInst %8: number
// CHECK-NEXT:function_end
