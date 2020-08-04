// Copyright (C) 2017 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.collator.prototype.constructor
description: >
  "constructor" property of Intl.Collator.prototype.
info: |
  Intl.Collator.prototype.constructor

  7 Requirements for Standard Built-in ECMAScript Objects

    Unless specified otherwise in this document, the objects, functions, and constructors
    described in this standard are subject to the generic requirements and restrictions
    specified for standard built-in ECMAScript objects in the ECMAScript 2018 Language
    Specification, 9th edition, clause 17, or successor.

  17 ECMAScript Standard Built-in Objects:

    Every other data property described in clauses 18 through 26 and in Annex B.2 has the
    attributes { [[Writable]]: true, [[Enumerable]]: false, [[Configurable]]: true }
    unless otherwise specified.

includes: [propertyHelper.js]
---*/

verifyNotEnumerable(Intl.Collator.prototype, "constructor");
verifyWritable(Intl.Collator.prototype, "constructor");
verifyConfigurable(Intl.Collator.prototype, "constructor");
