/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec -Xenable-tdz -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Xenable-tdz -O %s | %FileCheck --match-full-lines %s

// Verify code generation for a scoped for-loop.
// Verify IRGen logic for simplifying a scoped for-loop.

var arr = [];

// Worst possible case: every expression captures.
function foo_full() {
    for(let i = 0; arr.push(()=>i), ++i < 10; arr.push(()=>i), i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Test expression doesn't capture. This is the same as "full".
function foo_testnc() {
    for(let i = 0; ++i < 10; arr.push(()=>i), i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Update expression doesn't capture.
function foo_updatenc() {
    for(let i = 0; arr.push(()=>i), ++i < 10; i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Both test and update expressions don't capture.
function foo_testnc_updatenc() {
    for(let i = 0; ++i < 10; i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Nothing captures. This should be very similar to the var case, except for TDZ.
function foo_allnc() {
    for(let i = 0; ++i < 10; i += 2) {
        print(i);
    }
}

// The var case, which produces the best possible code.
function foo_var() {
    for(var i = 0; ++i < 10; i += 2) {
        print(i);
    }
}


function test(foo) {
    print(foo.name + ':');
    arr = [];
    foo();
    print("length", arr.length);
    // TOOD: when loop capturing scope is fixed
    // for(let f in arr) print(f());
};
test(foo_full);
test(foo_testnc);
test(foo_updatenc);
test(foo_testnc_updatenc);
test(foo_allnc);
test(foo_var);

//CHECK:      foo_full:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 10
//CHECK-NEXT: foo_testnc:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 6
//CHECK-NEXT: foo_updatenc:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 7
//CHECK-NEXT: foo_testnc_updatenc:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 3
//CHECK-NEXT: foo_allnc:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 0
//CHECK-NEXT: foo_var:
//CHECK-NEXT: 1
//CHECK-NEXT: 4
//CHECK-NEXT: 7
//CHECK-NEXT: length 0
