function assertArray(l, r) {
  assert(compareArray(l, r), r);
}

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/preferred-grandfathered.js
var irregularGrandfathered = [
  "en-gb-oed",
  "i-ami",
  "i-bnn",
  "i-default",
  "i-enochian",
  "i-hak",
  "i-klingon",
  "i-lux",
  "i-mingo",
  "i-navajo",
  "i-pwn",
  "i-tao",
  "i-tay",
  "i-tsu",
  "sgn-be-fr",
  "sgn-be-nl",
  "sgn-ch-de",
];

var regularGrandfatheredNonUTS35 = [
  "no-bok",
  "no-nyn",
  "zh-min",
  "zh-min-nan",
];

var regularGrandfatheredUTS35 = {
  "art-lojban": "jbo",
  "cel-gaulish": "xtg-x-cel-gaulish",
  //"zh-guoyu": "zh",
  "zh-hakka": "hak",
  "zh-xiang": "hsn",
};


irregularGrandfathered.forEach(function (tag) {
  assert.sameValue(
    isCanonicalizedStructurallyValidLanguageTag(tag), false,
    "Test data \"" + tag + "\" is not a structurally valid language tag."
  );
});

regularGrandfatheredNonUTS35.forEach(function (tag) {
  assert.sameValue(
    isCanonicalizedStructurallyValidLanguageTag(tag), false,
    "Test data \"" + tag + "\" is not a structurally valid language tag."
  );
});
Object.getOwnPropertyNames(regularGrandfatheredUTS35).forEach(function (tag) {
  var canonicalizedTag = regularGrandfatheredUTS35[tag];
  assert(
    isCanonicalizedStructurallyValidLanguageTag(canonicalizedTag),
    "Test data \"" + canonicalizedTag + "\" is a canonicalized and structurally valid language tag."
  );
});

Object.getOwnPropertyNames(regularGrandfatheredUTS35).forEach(function (tag) {
  var canonicalLocales = Intl.getCanonicalLocales(tag);
  assert.sameValue(canonicalLocales.length, 1);
  assert.sameValue(canonicalLocales[0], regularGrandfatheredUTS35[tag]);
});


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/preferred-variant.js
var canonicalizedTags = {
  // "ja-latn-hepburn-heploc": "ja-Latn-alalc97-hepburn", // TODO::Hermes icu4j reurns ja-latn-hepburn-heploc.. while icu4c returns ja-Latn-alalc97-hepburn .. Not V8 parity.
};

// make sure the data above is correct
Object.getOwnPropertyNames(canonicalizedTags).forEach(function (tag) {
  var canonicalizedTag = canonicalizedTags[tag];
  assert(
    isCanonicalizedStructurallyValidLanguageTag(canonicalizedTag),
    "Test data \"" + canonicalizedTag + "\" is not canonicalized and structurally valid language tag."
  );
});

Object.getOwnPropertyNames(canonicalizedTags).forEach(function (tag) {
  var canonicalLocales = Intl.getCanonicalLocales(tag);
  assert.sameValue(canonicalLocales.length, 1);
  assert.sameValue(canonicalLocales[0], canonicalizedTags[tag]);
});


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/returned-object-is-an-array.js

var locales = ['en-US'];
var result = Intl.getCanonicalLocales(['en-US']);

assert.sameValue(Object.getPrototypeOf(result), Array.prototype, 'prototype is Array.prototype');
assert.sameValue(result.constructor, Array);

assert.notSameValue(result, locales, "result is a new array instance");
assert.sameValue(result.length, 1, "result.length");
assert(result.hasOwnProperty("0"), "result an own property `0`");
assert.sameValue(result[0], "en-US", "result[0]");



// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/returned-object-is-mutable.js

var locales = ['en-US', 'fr'];
var result = Intl.getCanonicalLocales(locales);

verifyEnumerable(result, 0);
verifyWritable(result, 0);
verifyConfigurable(result, 0);

result = Intl.getCanonicalLocales(locales);
verifyEnumerable(result, 1);
verifyWritable(result, 1);
verifyConfigurable(result, 1);

result = Intl.getCanonicalLocales(locales);
verifyNotEnumerable(result, 'length');
verifyNotConfigurable(result, 'length');

assert.sameValue(result.length, 2);
result.length = 42;
assert.sameValue(result.length, 42);
assert.throws(RangeError, function() {
  result.length = "Leo";
}, "a non-numeric value can't be set to result.length");


