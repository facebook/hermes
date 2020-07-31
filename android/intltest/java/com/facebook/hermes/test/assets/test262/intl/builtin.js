// Copyright 2012 Mozilla Corporation. All rights reserved.
// This code is governed by the license found in the LICENSE file.

/*---
es5id: 8.0_L15
description: >
    Tests that Intl meets the requirements for built-in objects
    defined by the introduction of chapter 17 of the ECMAScript
    Language Specification.
author: Norbert Lindenberg
---*/

assert.sameValue(Object.prototype.toString.call(Intl), "[object Object]",
                 "The [[Class]] internal property of a built-in non-function object must be " +
                 "\"Object\".");

assert(Object.isExtensible(Intl), "Built-in objects must be extensible.");

assert.sameValue(Object.getPrototypeOf(Intl), Object.prototype,
                 "The [[Prototype]] of Intl is %ObjectPrototype%.");

assert.sameValue(this.Intl, Intl,
                 "%Intl% is accessible as a property of the global object.");
