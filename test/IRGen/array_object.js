/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function foo(param)
//CHECK-NEXT:frame = [obj, foo, param]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [obj]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [foo]
//CHECK-NEXT:  %2 = StoreFrameInst %param, [param]
//CHECK-NEXT:  %3 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %3 : object, "1" : string, true : boolean
//CHECK-NEXT:  %5 = LoadFrameInst [param]
//CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5, %3 : object, "key" : string, true : boolean
//CHECK-NEXT:  %7 = StoreFrameInst %3 : object, [obj]
//CHECK-NEXT:  %8 = AllocArrayInst 4 : number, 1 : number, 2 : number, 3 : number, 4 : number
//CHECK-NEXT:  %9 = StoreFrameInst %8 : object, [foo]
//CHECK-NEXT:  %10 = LoadFrameInst [obj]
//CHECK-NEXT:  %11 = LoadFrameInst [foo]
//CHECK-NEXT:  %12 = StorePropertyInst %11, %10, "field" : string
//CHECK-NEXT:  %13 = LoadFrameInst [foo]
//CHECK-NEXT:  %14 = LoadFrameInst [obj]
//CHECK-NEXT:  %15 = StorePropertyInst %14, %13, 5 : number
//CHECK-NEXT:  %16 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function foo(param) {
  var obj = {"1" : 2, "key" : param};

  var foo = [1,2,3,4];

  obj.field = foo;

  foo[5] = obj;
}
