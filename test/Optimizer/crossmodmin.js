// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//

// RUN: %hermes -enable-cpo -enable-xm=metromin -dump-ir %s -O | %FileCheck %s --match-full-lines

// mock up of bundling.
M = {};
global.__d = define;
function define(f, id) {
    function r(id) {
        var mod = M[id];
        return mod.exports;
    }
    var mod = {}
    f(r, mod);
    M[id] = mod;
};

__d(function(e,t,n,a) {
    // The function below is reported un-used in the statistics.
    n.exports = {a : 3, f : function() { return true; }}
}, 1);

__d(function(e,n,i,a) {
    i.exports = {b : n(1).a, f : n(1).f};
}, 2);

//CHECK-LABEL:function " 2#"(c, t, r, s) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:%1 = CallInst %t, undefined : undefined, 2 : number
//CHECK-NEXT:%2 = CallInst %0, undefined : undefined, 8 : number
//CHECK-NEXT:%3 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:%4 = StorePropertyInst %3 : object, %r, "exports" : string
//CHECK-NEXT:%5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
__d(function (c,t,r,s) {
    print(t(2).b + 5);  // rips out value from the object literal defined in mod 1
    r.exports={};
}, 3);
