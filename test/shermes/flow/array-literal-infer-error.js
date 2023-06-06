/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s
// XFAIL: true

(function() {
([1, "", false]: (number | bool)[])
// CHECK-LABEL: {{.*}}:11:6: error: ft: incompatible element type
// CHECK-NEXT:  ([1, "", false]: (number | bool)[])
// CHECK-NEXT:       ^~

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}

var derivedArr: Derived[] = [new Derived(), new Base()];
// CHECK-NEXT: {{.*}}:23:45: error: ft: incompatible element type
// CHECK-NEXT: var derivedArr: Derived[] = [new Derived(), new Base()];
// CHECK-NEXT:                                             ^~~~~~~~~~

var numArr: number[] = [1, 2, 3];
var strArr: string[] = ["a", "b", "c", ...numArr];
// CHECK-NEXT: {{.*}}:29:40: error: ft: incompatible element type
// CHECK-NEXT: var strArr: string[] = ["a", "b", "c", ...numArr];
// CHECK-NEXT:                                        ^~~~~~~~~
})();
