/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

(function() {
var str = "hello";
(str: number);
// CHECK-LABEL: {{.*}}:12:1: error: ft: cast from incompatible type
// CHECK-NEXT: (str: number);
// CHECK-NEXT: ^~~~~~~~~~~~~

var union: string | number = "";
(union: string);
// CHECK-NEXT: {{.*}}:18:1: error: ft: cast from incompatible type
// CHECK-NEXT: (union: string);
// CHECK-NEXT: ^~~~~~~~~~~~~~~

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}

var x: Base = new Base();
(x: Derived);
// CHECK-NEXT: {{.*}}:31:1: error: ft: cast from incompatible type
// CHECK-NEXT: (x: Derived);
// CHECK-NEXT: ^~~~~~~~~~~~
})();
