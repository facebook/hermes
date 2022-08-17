/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-bytecode %s | %FileCheck --match-full-lines %s

function foo() {
    var myNum = 1234;
    var myBool = true;
    var myString = 'a string';
    var myObj = new Object();
    var myRegExp = /Hermes/i;
    var myUndef = 'temp string';
    var myFunc = function bar(){
        myNum++;
        myBool = false;
        myString = 'new string';
        print(myObj);
        print(myRegExp);
        myUndef = undefined;
    }
    return myFunc;
}

// CHECK: Function<foo>(1 params, 10 registers, 6 symbols):
// CHECK-NEXT: Offset in debug table: {{.*}}
// CHECK-NEXT:     CreateEnvironment r0
// CHECK-NEXT:     LoadConstInt      r1, 1234
// CHECK-NEXT:     StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:     LoadConstTrue     r1
// CHECK-NEXT:     StoreNPToEnvironment r0, 1, r1
// CHECK-NEXT:     LoadConstString   r1, "a string"
// CHECK-NEXT:     StoreToEnvironment r0, 2, r1
// CHECK-NEXT:     GetGlobalObject   r1
// CHECK-NEXT:     TryGetById        r1, r1, 1, "Object"
// CHECK-NEXT:     GetByIdShort      r2, r1, 2, "prototype"
// CHECK-NEXT:     CreateThis        r2, r2, r1
// CHECK-NEXT:     Mov               r3, r2
// CHECK-NEXT:     Construct         r1, r1, 1
// CHECK-NEXT:     SelectObject      r1, r2, r1
// CHECK-NEXT:     StoreToEnvironment r0, 3, r1
// CHECK-NEXT:     CreateRegExp      r1, "Hermes", "i", 0
// CHECK-NEXT:     StoreToEnvironment r0, 4, r1
// CHECK-NEXT:     LoadConstString   r1, "temp string"
// CHECK-NEXT:     StoreToEnvironment r0, 5, r1
// CHECK-NEXT:     CreateClosure     r0, r0, Function<bar>
// CHECK-NEXT:     Ret               r0

// CHECK: Function<bar>(1 params, 13 registers, 0 symbols):
// CHECK-NEXT: Offset in debug table: {{.*}}
// CHECK-NEXT:    GetEnvironment    r1, 0
// CHECK-NEXT:    LoadFromEnvironment r0, r1, 0
// CHECK-NEXT:    Inc               r0, r0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r0
// CHECK-NEXT:    LoadConstFalse    r0
// CHECK-NEXT:    StoreNPToEnvironment r1, 1, r0
// CHECK-NEXT:    LoadConstString   r0, "new string"
// CHECK-NEXT:    StoreToEnvironment r1, 2, r0
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    TryGetById        r4, r2, 1, "print"
// CHECK-NEXT:    LoadFromEnvironment r3, r1, 3
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Call2             r3, r4, r0, r3
// CHECK-NEXT:    TryGetById        r3, r2, 1, "print"
// CHECK-NEXT:    LoadFromEnvironment r2, r1, 4
// CHECK-NEXT:    Call2             r2, r3, r0, r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 5, r0
// CHECK-NEXT:    Ret               r0
