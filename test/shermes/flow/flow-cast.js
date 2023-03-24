/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s

(function() {
var str: string = "hello";
(str: string | number);

class Base {}
class Derived extends Base {
    constructor(){
        super();
    }
}
var x: Derived = new Derived();
(x: Base);

type Alias = Base;
(x: Alias);
})();
