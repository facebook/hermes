/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Wno-direct-eval -O %s | %FileCheck --match-full-lines %s


// Test that local eval inherits the surrounding strictness.

(function (){
    print(typeof eval("(function(){return this;})()"));
})();
//CHECK: object

(function (){
    "use strict";
    print(typeof eval("(function(){return this;})()"));
})();
//CHECK: undefined
