/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s

var funcp;

// Make sure constructor is not optimized. Technically, using a function as
// new.target is problematic because it is an indirect reference to the
// constructor, and classes make tracking new.target worse due to inheritance.
// We could **try**, **really hard** to track all uses of a function as
// new.target, then see if in the possible uses new.target is
// referenced/stored/used as a function but this is possibly not worth (too
// time consuming for very little benefit).
(new function func(param) {
    try {
        funcp = new.target;
    } catch (e) { }
    let f = () => {
        return param;
    };
    return f;
})();

let f = new funcp(1);
print(f());
// CHECK: 1

f = new funcp(33);
print(f());
// CHECK: 33
