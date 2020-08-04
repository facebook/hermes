// Copyright (C) 2017 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.collator.prototype.compare
description: >
  get Intl.Collator.prototype.compare.length is 0.
info: |
  get Intl.Collator.prototype.compare

  17 ECMAScript Standard Built-in Objects:

    Every built-in function object, including constructors, has a length
    property whose value is an integer. Unless otherwise specified, this
    value is equal to the largest number of named arguments shown in the
    subclause headings for the function description. Optional parameters
    (which are indicated with brackets: [ ]) or rest parameters (which
    are shown using the form «...name») are not included in the default
    argument count.
    Unless otherwise specified, the length property of a built-in function
    object has the attributes { [[Writable]]: false, [[Enumerable]]: false,
    [[Configurable]]: true }.

includes: [propertyHelper.js]
---*/

var desc = Object.getOwnPropertyDescriptor(Intl.Collator.prototype, "compare");

assert.sameValue(desc.get.length, 0);

verifyNotEnumerable(desc.get, "length");
verifyNotWritable(desc.get, "length");
verifyConfigurable(desc.get, "length");
