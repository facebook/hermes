/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -max-register-stack=256 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-max-register-stack=256 %s | %FileCheck --match-full-lines %s

// Test cases of stack overflows where there is either JS mutual recursion or
// native C++ recursion in the interpreter. This checks that the handling for
// register stack overflows handles these cases correctly.

let fooCallee;

function foo(){
    fooCallee();
}

function test() {
    try {
        foo();
    } catch (e){
        print(e.message);
    }
}

// TODO: For single function recursion, we should be able to validate the top
//       few stack frames, but these currently differ between shermes and hermes.
// Test a simple foo-foo recursive call, which tests handling in tail calls in
// the interpreter.
print("fooCallee: foo")
fooCallee = foo;
test();

// CHECK: fooCallee: foo
// CHECK-NEXT: Maximum call stack size exceeded

// Test a call to foo.bind(), which forces a recursive interpreter invocation.
print("fooCallee: foo.bind()")
fooCallee = foo.bind();
test();
// CHECK: fooCallee: foo.bind()
// CHECK-NEXT: Maximum call stack size exceeded

// Test mutual recursion with different mixes of tail and recursive calls.
print("Mutual recursion");
let barCallee;
function bar(){
    barCallee();
}
for(fooCallee of [bar, bar.bind()]){
    for(barCallee of [foo, foo.bind()]){
        test();
    }
}

// CHECK: Mutual recursion
// CHECK-NEXT: Maximum call stack size exceeded
// CHECK-NEXT: Maximum call stack size exceeded
// CHECK-NEXT: Maximum call stack size exceeded
// CHECK-NEXT: Maximum call stack size exceeded
