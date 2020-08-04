// Copyright (C) 2017 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.collator.prototype.resolvedoptions
description: >
  "compare" property of Intl.Collator.prototype.
info: |
  get Intl.Collator.prototype.compare

  7 Requirements for Standard Built-in ECMAScript Objects

    Unless specified otherwise in this document, the objects, functions, and constructors
    described in this standard are subject to the generic requirements and restrictions
    specified for standard built-in ECMAScript objects in the ECMAScript 2018 Language
    Specification, 9th edition, clause 17, or successor.

  17 ECMAScript Standard Built-in Objects:

    Every accessor property described in clauses 18 through 26 and in Annex B.2 has the
    attributes { [[Enumerable]]: false, [[Configurable]]: true } unless otherwise specified.
    If only a get accessor function is described, the set accessor function is the default
    value, undefined. If only a set accessor is described the get accessor is the default
    value, undefined.

includes: [propertyHelper.js]
---*/

var desc = Object.getOwnPropertyDescriptor(Intl.Collator.prototype, "compare");

assert.sameValue(desc.set, undefined);
assert.sameValue(typeof desc.get, "function");

verifyNotEnumerable(Intl.Collator.prototype, "compare");
verifyConfigurable(Intl.Collator.prototype, "compare");
