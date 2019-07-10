// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -enable-cla -enable-umo -dump-ir %s -O -strict | %FileCheck %s --match-full-lines

//CHECK-LABEL:function unused() : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:%0 = ThrowInst "unused" : string
//CHECK-NEXT:function_end

//CHECK-LABEL:function escaped(p, q) : string|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:%0 = BinaryOperatorInst '+', %p, %q
//CHECK-NEXT:%1 = ReturnInst %0 : string|number
//CHECK-NEXT:function_end
function module1(module) {
    var o = {
        m : function unused() {  // removed by UMO
            var t = Date.now();
            print(t);
        },
        n : function escaped(p,q) {  // not removed due to escapement
            return p+q;
        }
    }
    module.exports=o.n;
}

//CHECK-LABEL:function f(x) : undefined
//CHECK-NEXT:frame = [v]
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = ThrowInst "f" : string
//CHECK-NEXT:function_end
function module2(module) {
    function f(x) {  // removed by UMO
        var v;
        function g() {  // removed by DCE
            v = v+ 1;
        }
    }
    var o = { m : f, n : 3 };
    return o.n;
}
