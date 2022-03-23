/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Check iterator closing in the presence of breaks and exceptions.
// Same as for-of-close-1.js, but with generators!

var throwInNext = false;
var throwInReturn = false;

function *o() {
    var cnt = 0;
    var normalReturn = false;
    var threw = false;
    try {
        for(;;) {
            print("next() called");
            ++cnt;
            if (cnt === throwInNext) {
                print("next() throwing");
                threw = true;
                throw "<from next()>";
            }
            if (cnt === 5)
                break;
            yield cnt + 10;
        }
        normalReturn = true;
    } finally {
        if (!normalReturn && !threw) {
            print("return() called");
            if (throwInReturn) {
                print("return() throwing");
                throw "<from return()>";
            }
        }
    }
}

// Leave the loop with break.
print("\ntest1");
try {
    for(let i of o()) {
        print(i);
        if (i === 12)
            break;
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test1
//CHECK-NEXT: next() called
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: 12
//CHECK-NEXT: return() called

// Leave the loop with break and throw in iterator.return()
throwInReturn = true;
print("\ntest2");
try {
    for(let i of o()) {
        print(i);
        if (i === 12)
            break;
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test2
//CHECK-NEXT: next() called
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: 12
//CHECK-NEXT: return() called
//CHECK-NEXT: return() throwing
//CHECK-NEXT: caught <from return()>

// Try leaving the loop with an exception.
throwInReturn = false
returnNonObject = false;
print("\ntest4");
try {
    for(let i of o()) {
        print(i);
        if (i === 12)
            throw "<from loop>";
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test4
//CHECK-NEXT: next() called
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: 12
//CHECK-NEXT: return() called
//CHECK-NEXT: caught <from loop>

// Try leaving the loop with an exception and also throw in the iterator return().
throwInReturn = true;
print("\ntest5");
try {
    for(let i of o()) {
        print(i);
        if (i === 12)
            throw "<from loop>";
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test5
//CHECK-NEXT: next() called
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: 12
//CHECK-NEXT: return() called
//CHECK-NEXT: return() throwing
//CHECK-NEXT: caught <from loop>


// Make sure exceptions in the lhs are handled.
function makeIndexer() {
    var cnt = 0;
    return function () {
        print("in indexer");
        if (++cnt == 3) {
            print("indexer throwing");
            throw "<from indexer>";
        }
        return 0;
    }
}

throwInReturn = false;
print("\ntest7");
try {
    var ar = []
    var indexer = makeIndexer();
    for(ar[indexer()] of o()) {
        print(ar[0]);
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test7
//CHECK-NEXT: next() called
//CHECK-NEXT: in indexer
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: in indexer
//CHECK-NEXT: 12
//CHECK-NEXT: next() called
//CHECK-NEXT: in indexer
//CHECK-NEXT: indexer throwing
//CHECK-NEXT: return() called
//CHECK-NEXT: caught <from indexer>

// Make sure exceptions thrown from next() are not handled.
throwInNext = 2;
print("\ntest8");
try {
    for(let i of o()) {
        print(i);
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test8
//CHECK-NEXT: next() called
//CHECK-NEXT: 11
//CHECK-NEXT: next() called
//CHECK-NEXT: next() throwing
//CHECK-NEXT: caught <from next()>

// Just sanity check that continue works fine.
throwInNext = false;
print("\ntest9");
try {
    for(let i of o()) {
        if (i & 1)
            continue;
        print(i);
    }
} catch (e) {
    print("caught", e);
}
//CHECK-LABEL: test9
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: 12
//CHECK-NEXT: next() called
//CHECK-NEXT: next() called
//CHECK-NEXT: 14
//CHECK-NEXT: next() called
