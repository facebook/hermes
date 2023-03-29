/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xenable-tdz -exec %s | %FileCheck --match-full-lines %s

function foo(f = 123) {
    print(f);
    {
        print(f);
        var f = 10;
        print(f);
    }
    print(f);
}

foo();

//CHECK:      123
//CHECK-NEXT: 123
//CHECK-NEXT: 10
//CHECK-NEXT: 10
