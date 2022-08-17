/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

// Check iterator closing in array destructoring with exceptions.

var o = {
    ofs: 10,
}

var throwInNext = false;
var throwInReturn = false;
var returnNonObject = false;

o[Symbol.iterator] = function() {
    var cnt = 0;
    var iter = {
        next: (arg) => {
            print("next() called");
            ++cnt;
            if (cnt === throwInNext) {
                print("next() throwing");
                throw "<from next()>";
            }
            if (cnt === 4) {
                return { done: true }
            }
            return { value: this.ofs + cnt }
        },
        return: (arg) => {
            print("return() called");
            if (throwInReturn) {
                print("return() throwing");
                throw "<from return()>";
            }
            return returnNonObject ? 0 : {}
        }
    }
    return iter;
}


// Iterator is done before the end.
print("\ntest1");
var [a, b, c, d, e] = o;
//CHECK-LABEL: test1
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called

// Iterator is not done at end.
print("\ntest2");
var [a, b] = o;
//CHECK-LABEL: test2
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: return() called

// Exception in lref
print("\ntest3");
function thr() {
    print("thr() throwing");
    throw "<from thr()>";
}
var arr = [];

try {
  [a, arr[thr()]] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test3
//CHECK-NEXT: next() called
//CHECK-NEXT: thr() throwing
//CHECK-NEXT: return() called
//CHECK-NEXT: caught <from thr()>

// Exception when storing to lref
print("\ntest4");
var badobj = {
    set prop(x) {
        print("badobj.prop() throwing");
        throw "<from badobj.prop()>";
    }
}

try {
  [a, badobj.prop] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test4
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: badobj.prop() throwing
//CHECK-NEXT: return() called
//CHECK-NEXT: caught <from badobj.prop()>

// Make sure that exceptions thrown by next() are not caught.
print("\ntest5");
throwInNext = 2;
try {
  [a, b, c] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test5
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() throwing
//CHECK-NEXT: caught <from next()>

// Make sure that return is not called if an exception is thrown
// after the iterator is done.
print("\ntest6");
throwInNext = false;
try {
    [a, b, c, d, a, arr[thr()]] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test6
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: thr() throwing
//CHECK-NEXT: caught <from thr()>

// Make sure that exceptions in the rest element cause closing.
print("\ntest7");
throwInNext = 3;
try {
    [...arr[thr()]] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test7
//CHECK-NEXT: thr() throwing
//CHECK-NEXT: return() called
//CHECK-NEXT: caught <from thr()>

// Exception when storing to lref in the rest element
print("\ntest8");
throwInNext = false;
try {
  [...badobj.prop] = o;
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test8
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: badobj.prop() throwing
//CHECK-NEXT: caught <from badobj.prop()>
