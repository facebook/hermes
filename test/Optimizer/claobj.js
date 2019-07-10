// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -enable-cla -dump-ir %s -O | %FileCheck %s --match-full-lines

//CHECK-LABEL:function g() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%1 = CreateFunctionInst %m() : number
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %1 : closure, %0 : object, "m" : string, true : boolean
//CHECK-NEXT:%3 = LoadPropertyInst %0 : object, "m" : string
//CHECK-NEXT:%4 = CallInst %3 : closure, %0 : object
//CHECK-NEXT:%5 = ReturnInst %4 : number
//CHECK-NEXT:function_end
function module1() {
    function g() {
        var o = {
            m: function () {
                return 1;
            }
        };
        var p = o.m();
        return p;
    }

     return g(); // finds number
}

//CHECK-LABEL:function gg() : boolean
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %h() : object
//CHECK-NEXT:%1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:%2 = LoadPropertyInst %1 : object, "m" : string
//CHECK-NEXT:%3 = CallInst %2 : closure, %1 : object
//CHECK-NEXT:%4 = ReturnInst %3 : boolean
//CHECK-NEXT:function_end
function module2() {
    function gg() {
        function h() {
            var o = { m: function () {
                    return false;
                }
            };
            return o;
        }
        return h().m();
    }
    return gg();  // finds boolean
}

//CHECK-LABEL:function module3() : boolean
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %hh() : closure
//CHECK-NEXT:%1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:%2 = CallInst %1 : closure, undefined : undefined
//CHECK-NEXT:%3 = ReturnInst %2 : boolean
//CHECK-NEXT:function_end
function module3() {
    function hh() {
        var o = { x : { y : function () { return true; }}};
        return o.x.y;
    }
    return hh()(); // finds boolean
}


function bad(o) {}
//CHECK-LABEL:function module4()
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %"h 1#"() : object
//CHECK-NEXT:%1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:%2 = LoadPropertyInst globalObject : object, "bad" : string
//CHECK-NEXT:%3 = CallInst %2, undefined : undefined, %1 : object
//CHECK-NEXT:%4 = LoadPropertyInst %1 : object, "m" : string
//CHECK-NEXT:%5 = CallInst %4, %1 : object
//CHECK-NEXT:%6 = ReturnInst %5
//CHECK-NEXT:function_end
function module4() {
    function h() {
        var o = { m: function () {
            return false;
        }
        };
        return o;
    }
    var o = h();
    bad(o);
    return o.m(); // must not infer v is boolean due to 'bad' above
}

//CHECK-LABEL:function module5() : closure
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %"h 2#"() : object
//CHECK-NEXT:%1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:%2 = LoadPropertyInst %1 : object, "m" : string
//CHECK-NEXT:%3 = CallInst %2, %1 : object
//CHECK-NEXT:%4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:%5 = CallInst %4, undefined : undefined, %3
//CHECK-NEXT:%6 = ReturnInst %0 : closure
//CHECK-NEXT:function_end
function module5() {
    function h() {
        var o = { m: function () {
            return false;
        }
        };
        return o;
    }
    var o = h();
    var v = o.m();
    print (v); // must not infer v is boolean
    return h; // h escapes, so o escapes
}

//CHECK-LABEL:function module6() : boolean|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%1 = CreateFunctionInst %n() : boolean
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %1 : closure, %0 : object, "n" : string, true : boolean
//CHECK-NEXT:%3 = CreateFunctionInst %"m 4#"() : undefined
//CHECK-NEXT:%4 = StoreNewOwnPropertyInst %3 : closure, %0 : object, "m" : string, true : boolean
//CHECK-NEXT:%5 = LoadPropertyInst %0 : object, "m" : string
//CHECK-NEXT:%6 = CallInst %5 : closure, %0 : object
//CHECK-NEXT:%7 = LoadPropertyInst %0 : object, "n" : string
//CHECK-NEXT:%8 = CallInst %7 : closure, %0 : object
//CHECK-NEXT:%9 = ReturnInst %8 : boolean|number
//CHECK-NEXT:function_end
function module6() {
    var o = {
        n : function () { return false; },
        m : function () { this.n = function () { return 1; }; }
    }
    o.m();
    var v = o.n();
    return v;
}


//CHECK-LABEL:function module7() : boolean
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%1 = CreateFunctionInst %a() : boolean
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %1 : closure, %0 : object, "a" : string, true : boolean
//CHECK-NEXT:%3 = CreateFunctionInst %"m 5#"() : closure
//CHECK-NEXT:%4 = StoreNewOwnPropertyInst %3 : closure, %0 : object, "m" : string, true : boolean
//CHECK-NEXT:%5 = LoadPropertyInst %0 : object, "m" : string
//CHECK-NEXT:%6 = CallInst %5 : closure, %0 : object
//CHECK-NEXT:%7 = CallInst %6 : closure, undefined : undefined
//CHECK-NEXT:%8 = ReturnInst %7 : boolean
//CHECK-NEXT:function_end
function module7() {
    var o = { a : function () { return true; },
        m : function () { return this.a; }};
    return o.m()();  // returns boolean
}

//CHECK-LABEL:function module8()
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = CreateFunctionInst %f() : boolean
//CHECK-NEXT:%1 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:%2 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%3 = StoreNewOwnPropertyInst %0 : closure, %2 : object, "m" : string, true : boolean
//CHECK-NEXT:%4 = StorePropertyInst %2 : object, %1 : object, "foo" : string
//CHECK-NEXT:%5 = LoadPropertyInst %1 : object, "foo" : string
//CHECK-NEXT:%6 = LoadPropertyInst %5, "m" : string
//CHECK-NEXT:%7 = CallInst %6, %5
//CHECK-NEXT:%8 = ReturnInst %7
//CHECK-NEXT:function_end
function module8() {
    var f = function () { return false; }
    var o = {};
    o.foo = { m : f }; // foo is not owned.
    return (o.foo.m)();  // we should not infer boolean here.
}

//CHECK-LABEL:function module9() : boolean
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%1 = CreateFunctionInst %"a 1#"() : boolean
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %1 : closure, %0 : object, "a" : string, true : boolean
//CHECK-NEXT:%3 = CreateFunctionInst %b() : boolean
//CHECK-NEXT:%4 = StoreNewOwnPropertyInst %3 : closure, %0 : object, "b" : string, true : boolean
//CHECK-NEXT:%5 = LoadPropertyInst %0 : object, "b" : string
//CHECK-NEXT:%6 = CallInst %5 : closure, %0 : object
//CHECK-NEXT:%7 = ReturnInst %6 : boolean
//CHECK-NEXT:function_end
function module9() {
    var o = {
        a: function (x) {
            return true;
        },
        b: function () {
            var v = this.a(1);
            return v;
        }
    };
    return o.b();
}

//CHECK-LABEL:function module13(c) : boolean|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%1 = CreateFunctionInst %num() : number
//CHECK-NEXT:%2 = StoreNewOwnPropertyInst %1 : closure, %0 : object, "num" : string, true : boolean
//CHECK-NEXT:%3 = StoreNewOwnPropertyInst undefined : undefined, %0 : object, "next" : string, true : boolean
//CHECK-NEXT:%4 = CondBranchInst %c, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:%5 = PhiInst %0 : object, %BB0, %11 : undefined|object, %BB1
//CHECK-NEXT:%6 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:%7 = CreateFunctionInst %"num 1#"() : boolean
//CHECK-NEXT:%8 = StoreNewOwnPropertyInst %7 : closure, %6 : object, "num" : string, true : boolean
//CHECK-NEXT:%9 = StoreNewOwnPropertyInst undefined : undefined, %6 : object, "next" : string, true : boolean
//CHECK-NEXT:%10 = StorePropertyInst %6 : object, %5 : undefined|object, "next" : string
//CHECK-NEXT:%11 = LoadPropertyInst %5 : undefined|object, "next" : string
//CHECK-NEXT:%12 = CondBranchInst %c, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:%13 = PhiInst %0 : object, %BB0, %11 : undefined|object, %BB1
//CHECK-NEXT:%14 = LoadPropertyInst %13 : undefined|object, "num" : string
//CHECK-NEXT:%15 = CallInst %14 : closure, %13 : undefined|object
//CHECK-NEXT:%16 = ReturnInst %15 : boolean|number
//CHECK-NEXT:function_end
function module13(c) { // Illustrates recursive structure
    // It is important to give next as an owned property.
    var heap = { num : function () { return 1;}, next : undefined};

    while (c) {
        heap.next = { num : function() { return false;}, next : undefined};
        heap = heap.next;
    }

    return heap.num(); // finds boolean|number
}
