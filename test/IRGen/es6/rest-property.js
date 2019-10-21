/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s

function f1(t) {
    var {...a} = t;
    return a;
}
//CHECK-LABEL: function f1(t)
//CHECK-NEXT: frame = [a, t]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:   %1 = StoreFrameInst %t, [t]
//CHECK-NEXT:   %2 = LoadFrameInst [t]
//CHECK-NEXT:   %3 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:   %4 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %5 = LoadPropertyInst %4, "copyDataProperties" : string
//CHECK-NEXT:   %6 = CallInst %5, undefined : undefined, %3 : object, %2, undefined : undefined
//CHECK-NEXT:   %7 = StoreFrameInst %6, [a]
//CHECK-NEXT:   %8 = LoadFrameInst [a]
//CHECK-NEXT:   %9 = ReturnInst %8
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %10 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

function f2(t) {
    var {a, b, ...rest} = t;
    return rest;
}
//CHECK-LABEL: function f2(t)
//CHECK-NEXT: frame = [a, b, rest, t]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [b]
//CHECK-NEXT:   %2 = StoreFrameInst undefined : undefined, [rest]
//CHECK-NEXT:   %3 = StoreFrameInst %t, [t]
//CHECK-NEXT:   %4 = LoadFrameInst [t]
//CHECK-NEXT:   %5 = LoadPropertyInst %4, "a" : string
//CHECK-NEXT:   %6 = StoreFrameInst %5, [a]
//CHECK-NEXT:   %7 = LoadPropertyInst %4, "b" : string
//CHECK-NEXT:   %8 = StoreFrameInst %7, [b]
//CHECK-NEXT:   %9 = HBCAllocObjectFromBufferInst 2 : number, "a" : string, 0 : number, "b" : string, 0 : number
//CHECK-NEXT:   %10 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:   %11 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %12 = LoadPropertyInst %11, "copyDataProperties" : string
//CHECK-NEXT:   %13 = CallInst %12, undefined : undefined, %10 : object, %4, %9 : object
//CHECK-NEXT:   %14 = StoreFrameInst %13, [rest]
//CHECK-NEXT:   %15 = LoadFrameInst [rest]
//CHECK-NEXT:   %16 = ReturnInst %15
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %17 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

function f3(t) {
    var a, rest;
    ({a, ...rest} = t);
}
//CHECK-LABEL: function f3(t)
//CHECK-NEXT: frame = [a, rest, t]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [rest]
//CHECK-NEXT:   %2 = StoreFrameInst %t, [t]
//CHECK-NEXT:   %3 = LoadFrameInst [t]
//CHECK-NEXT:   %4 = LoadPropertyInst %3, "a" : string
//CHECK-NEXT:   %5 = StoreFrameInst %4, [a]
//CHECK-NEXT:   %6 = HBCAllocObjectFromBufferInst 1 : number, "a" : string, 0 : number
//CHECK-NEXT:   %7 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:   %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %9 = LoadPropertyInst %8, "copyDataProperties" : string
//CHECK-NEXT:   %10 = CallInst %9, undefined : undefined, %7 : object, %3, %6 : object
//CHECK-NEXT:   %11 = StoreFrameInst %10, [rest]
//CHECK-NEXT:   %12 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

function f4(o, t) {
    var a;
    ({a, ...o.rest} = t);
}
//CHECK-LABEL: function f4(o, t)
//CHECK-NEXT: frame = [a, o, t]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [a]
//CHECK-NEXT:   %1 = StoreFrameInst %o, [o]
//CHECK-NEXT:   %2 = StoreFrameInst %t, [t]
//CHECK-NEXT:   %3 = LoadFrameInst [t]
//CHECK-NEXT:   %4 = LoadPropertyInst %3, "a" : string
//CHECK-NEXT:   %5 = StoreFrameInst %4, [a]
//CHECK-NEXT:   %6 = LoadFrameInst [o]
//CHECK-NEXT:   %7 = HBCAllocObjectFromBufferInst 1 : number, "a" : string, 0 : number
//CHECK-NEXT:   %8 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:   %9 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:   %10 = LoadPropertyInst %9, "copyDataProperties" : string
//CHECK-NEXT:   %11 = CallInst %10, undefined : undefined, %8 : object, %3, %7 : object
//CHECK-NEXT:   %12 = StorePropertyInst %11, %6, "rest" : string
//CHECK-NEXT:   %13 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
