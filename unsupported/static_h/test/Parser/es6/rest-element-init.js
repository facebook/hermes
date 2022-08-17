/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

function trycode(code) {
    try {
        (0,eval)(code);
    } catch (e) {
        print("caught:", e.message);
        return;
    }
    print("OK");
}

print("BEGIN")
//CHECK: BEGIN

trycode("function foo(a, ...b = 10) {}");
//CHECK-NEXT: caught: 1:20:rest elemenent may not have a default initializer

trycode("let foo = (a, ...b = 10) => {}");
//CHECK-NEXT: caught: 1:18:rest elemenent may not have a default initializer

trycode("var [a, ...b = []] = []");
//CHECK-NEXT: caught: 1:12:rest elemenent may not have a default initializer

trycode("[a, ...b = []] = []");
//CHECK-NEXT: caught: 1:8:invalid assignment left-hand side

trycode("var {a, ...b = {}} = {}");
//CHECK-NEXT: caught: 1:14:'}' expected at end of object binding pattern '{...'

trycode("({a, ...b = {}} = {})");
//CHECK-NEXT: caught: 1:9:invalid assignment left-hand side
