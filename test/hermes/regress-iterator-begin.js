/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// The iterator's 'next' method is not a Callable.
// However, that shouldn't matter since the array destructuring is empty
// and so 'next' should never be called.

"use strict";

var o = {
    [Symbol.iterator]:  function() {
        return {
            next: 10,
            return: function() { 
                print("return"); 
                return {done: true};
            }
        }
    }
}

function foo() {
  var [] = o;
}

foo();

// CHECK: return
