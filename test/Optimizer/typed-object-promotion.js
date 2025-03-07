/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

class Foo{
  x: number;
  y: number;
  constructor(x: number, y: number){
    "inline";
    this.x = x;
    this.y = y;
  }
}

function makeFoo(): Foo{
  return new Foo(1, 2);
}
var foo: Foo = makeFoo();
print(foo.x, foo.y);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %Foo(): functionCode
// CHECK-NEXT:  %1 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %0: object, "prototype": string
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 1: number, 2: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor Foo(x: number, y: number): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %x: number
// CHECK-NEXT:  %2 = LoadParamInst (:number) %y: number
// CHECK-NEXT:       PrStoreInst %1: number, %0: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:       PrStoreInst %2: number, %0: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
