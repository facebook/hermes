/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Ensure that eval() generates correct error locations.

print("START");
//CHECK: START

var global = Function("return this")();

try{ global.eval(" )"); } catch (e) {
    print(e.message);
}
//CHECK-NEXT: 1:2:invalid expression

try{ global.eval("throw new Error()\n//# sourceURL=foo"); } catch (e) {
    print(e.stack);
}
//CHECK-NEXT: Error
//CHECK-NEXT:   at global (foo:1:16)

try {
    var f = new Function(" 'use strict'; var o = {}; with (o) {}; ");
} catch (e) {
    print(e.stack);
}
//CHECK: SyntaxError: 1:41:invalid statement encountered.
//CHECK-NEXT:     at Function (native)
//CHECK-NEXT:     at global ({{.*}}eval-errors.js:{{.*}}:25)
