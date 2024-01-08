/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

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
