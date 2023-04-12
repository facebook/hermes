/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s

(function() {
// The resulting type is a union of its elements.
var unionArr = [1, "", undefined];

// Assigning to an annotated variable lets us use its type.
var annotatedArr: (string | number | void)[] = [1, ""];

// The two arrays have the same type.
annotatedArr = unionArr

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}

var baseArr: Base[] = [new Base(), new Derived()];

var numArr: number[] = [1, 2, 3];
var strArr: (number | string)[] = ["a", "b", "c", ...numArr];

var anyArr: any[] = [];
([...anyArr]: number[]);
})();
