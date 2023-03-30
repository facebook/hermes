/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xenable-tdz -exec %s | %FileCheck --match-full-lines %s

function foo(f = 123) {
    print(typeof f);
    {
        print(typeof f);
        function f() {}
        print(typeof f);
    }
    print(typeof f);
}

foo();

//CHECK:      number
//CHECK-NEXT: function
//CHECK-NEXT: function
//CHECK-NEXT: number
