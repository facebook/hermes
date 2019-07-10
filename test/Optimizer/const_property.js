// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -enable-cla -enable-cpo -dump-ir %s -O -fno-inline | %FileCheck %s --match-full-lines

"use strict";

//CHECK-LABEL:function module1() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%1 = StoreNewOwnPropertyInst 1 : number, %0 : object, "a" : string, true : boolean
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst 2 : number, %0 : object, "b" : string, true : boolean
//CHECK-NEXT:%3 = ReturnInst 3 : number
//CHECK-NEXT:function_end
function module1() {
  var o = { a : 1, b : 2 };
  return o.a + o.b;
}

//CHECK-LABEL:function module2(module) : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %f() : object
//CHECK-NEXT:%1 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst 3 : number, %1 : object, "v" : string, true : boolean
//CHECK-NEXT:%3 = CallInst %0 : closure, undefined : undefined, %1 : object
//CHECK-NEXT:%4 = ReturnInst 3 : number
//CHECK-NEXT:function_end
function module2(module) {
    var f = function (x) { return x; }
    return f({v:3}).v;
}

// The next three tests show that when f escapes, const prop is inhibited.

//CHECK-LABEL:function module2a(module)
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %"f 1#"()
//CHECK-NEXT:%1 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %0 : closure, %1 : object, "ff" : string, true : boolean
//CHECK-NEXT:%3 = StorePropertyInst %1 : object, %module, "exports" : string
//CHECK-NEXT:%4 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%5 = StoreNewOwnPropertyInst 3 : number, %4 : object, "v" : string, true : boolean
//CHECK-NEXT:%6 = CallInst %0 : closure, undefined : undefined, %4 : object
//CHECK-NEXT:%7 = LoadPropertyInst %4 : object, "v" : string
//CHECK-NEXT:%8 = ReturnInst %7
//CHECK-NEXT:function_end
function module2a(module) {
    var f = function (x) { return x; }
    module.exports = { ff : f }; // f escapes, suppresses return 3
    return f({v:3}).v;
}

//CHECK-LABEL:function module2b()
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %"f 2#"()
//CHECK-NEXT:%1 = LoadPropertyInst %0 : closure, "bind" : string
//CHECK-NEXT:%2 = CallInst %1, %0 : closure, 2 : number
//CHECK-NEXT:%3 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%4 = StoreNewOwnPropertyInst 3 : number, %3 : object, "v" : string, true : boolean
//CHECK-NEXT:%5 = CallInst %0 : closure, undefined : undefined, %3 : object
//CHECK-NEXT:%6 = LoadPropertyInst %3 : object, "v" : string
//CHECK-NEXT:%7 = ReturnInst %6
//CHECK-NEXT:function_end
function module2b() {
    var f = function (x) { return x; }
    f.bind(2); // f escapes, suppresses return 3
    return f({v:3}).v;
}

//CHECK-LABEL:function module2c()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %"f 3#"()
//CHECK-NEXT:  %1 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:  %2 = StoreNewOwnPropertyInst %0 : closure, %1 : object, "some" : string, true : boolean
//CHECK-NEXT:  %3 = LoadPropertyInst %1 : object, "other" : string
//CHECK-NEXT:  %4 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %4 : object, "v" : string, true : boolean
//CHECK-NEXT:  %6 = CallInst %0 : closure, undefined : undefined, %4 : object
//CHECK-NEXT:  %7 = LoadPropertyInst %4 : object, "v" : string
//CHECK-NEXT:  %8 = ReturnInst %7
//CHECK-NEXT:function_end
function module2c() {
    var f = function (x) { return x; };
    ({ some : f })["other"]; //  f escapes, suppresses return 3
    return f({v:3}).v;
}

//CHECK-LABEL:function module4d() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %"f 4#"() : object
//CHECK-NEXT:%1 = LoadPropertyInst %0 : closure, "prototype" : string
//CHECK-NEXT:%2 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%3 = StoreNewOwnPropertyInst 3 : number, %2 : object, "v" : string, true : boolean
//CHECK-NEXT:%4 = CallInst %0 : closure, undefined : undefined, %2 : object
//CHECK-NEXT:%5 = ReturnInst 3 : number
//CHECK-NEXT:function_end
function module4d() {
    var f = function (x) { return x; }
    var g = f.prototype; // do not escape by read of prototype
    return f({v:3}).v;
}
