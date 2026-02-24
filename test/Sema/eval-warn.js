/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s 2>&1 >/dev/null | %FileCheck --match-full-lines %s


function foo() {
    return eval("1 + 1");
//CHECK: {{.*}}eval-warn.js:12:12: warning: Direct call to eval(), but lexical scope is not supported.
//CHECK-NEXT:     return eval("1 + 1");
//CHECK-NEXT:            ^~~~

}

function bar() {
    return eval("2 + 2", Math, foo());
//CHECK: {{.*}}eval-warn.js:20:12: warning: Direct call to eval(), but lexical scope is not supported.
//CHECK-NEXT:     return eval("2 + 2", Math, foo());
//CHECK-NEXT:            ^~~~
}
