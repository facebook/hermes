// Copyright 2012 Mozilla Corporation. All rights reserved.
// This code is governed by the license found in the LICENSE file.

/*---
es5id: 10.3.3
description: >
    Tests that the object returned by
    Intl.Collator.prototype.resolvedOptions  has the right properties.
author: Norbert Lindenberg
includes: [testIntl.js, propertyHelper.js]
---*/

function stringify(obj) {
    if (typeof obj !== 'object' || obj === null || obj instanceof Array) {
        return value(obj);
    }

    return '{' + Object.keys(obj).map(function (k) {
        return (typeof obj[k] === 'function') ? null : '"' + k + '":' + value(obj[k]);
    }).filter(function (i) { return i; }) + '}';
}

function value(val) {
    switch(typeof val) {
        case 'string':
            return '"' + val.replace(/\\/g, '\\\\').replace('"', '\\"') + '"';
        case 'number':
        case 'boolean':
            return '' + val;
        case 'function':
            return 'null';
        case 'object':
            if (val instanceof Date)  return '"' + val.toISOString() + '"';
            if (val instanceof Array) return '[' + val.map(value).join(',') + ']';
            if (val === null)         return 'null';
                                      return stringify(val);
    }
}

var actual = new Intl.Collator().resolvedOptions();
print("Stringified actual: " + stringify(actual));

var actual2 = new Intl.Collator().resolvedOptions();
assert.notSameValue(actual2, actual, "resolvedOptions returned the same object twice.");

// source: CLDR file common/bcp47/collation.xml; version CLDR 32.
var collations = [
    "default", // added
    "big5han",
    "compat",
    "dict",
    "direct",
    "ducet",
    "emoji",
    "eor",
    "gb2312",
    "phonebk",
    "phonetic",
    "pinyin",
    "reformed",
    // "search", // excluded
    "searchjl",
    // "standard", // excluded
    "stroke",
    "trad",
    "unihan",
    "zhuyin",
];

// this assumes the default values where the specification provides them
assert(isCanonicalizedStructurallyValidLanguageTag(actual.locale),
       "Invalid locale: " + actual.locale);
assert.sameValue(actual.usage, "sort");
assert.sameValue(actual.sensitivity, "variant");
assert.sameValue(actual.ignorePunctuation, false);
assert.notSameValue(collations.indexOf(actual.collation), -1,
                    "Invalid collation: " + actual.collation);

var dataPropertyDesc = { writable: true, enumerable: true, configurable: true };
verifyProperty(actual, "locale", dataPropertyDesc);
verifyProperty(actual, "usage", dataPropertyDesc);
verifyProperty(actual, "sensitivity", dataPropertyDesc);
verifyProperty(actual, "ignorePunctuation", dataPropertyDesc);
verifyProperty(actual, "collation", dataPropertyDesc);

// "numeric" is an optional property.
if (actual.hasOwnProperty("numeric")) {
    assert.notSameValue([true, false].indexOf(actual.numeric), -1);
    verifyProperty(actual, "numeric", dataPropertyDesc);
}

// "caseFirst" is an optional property.
if (actual.hasOwnProperty("caseFirst")) {
    assert.notSameValue(["upper", "lower", "false"].indexOf(actual.caseFirst), -1);
    verifyProperty(actual, "caseFirst", dataPropertyDesc);
}
