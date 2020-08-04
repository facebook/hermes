// Copyright (C) 2017 André Bargull. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.collator.supportedlocalesof
description: >
  Intl.Collator.supportedLocalesOf.length is 1.
info: |
  Intl.Collator.supportedLocalesOf ( locales [ , options ] )

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

assert.sameValue(Intl.Collator.supportedLocalesOf.length, 1);

verifyNotEnumerable(Intl.Collator.supportedLocalesOf, "length");
verifyNotWritable(Intl.Collator.supportedLocalesOf, "length");
verifyConfigurable(Intl.Collator.supportedLocalesOf, "length");
