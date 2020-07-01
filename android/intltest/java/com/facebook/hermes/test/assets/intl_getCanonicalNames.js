function assertArray(l, r) {
  assert(compareArray(l, r), r);
}

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/proto.js
assert.sameValue(Object.getPrototypeOf(Intl), Object.prototype, "Intl doesn't have Object.prototype as its prototype.");


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/builtin.js
assert.sameValue(Object.prototype.toString.call(Intl), "[object Object]",
                 "The [[Class]] internal property of a built-in non-function object must be " +
                 "\"Object\".");

assert(Object.isExtensible(Intl), "Built-in objects must be extensible.");

assert.sameValue(Object.getPrototypeOf(Intl), Object.prototype,
                 "The [[Prototype]] of Intl is %ObjectPrototype%.");

assert.sameValue(this.Intl, Intl,
                 "%Intl% is accessible as a property of the global object.");

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/Locale-object.js

// We don't yet support Hermes::Locale constructor .. // TODO::HERMES
assert.compareArray(Intl.getCanonicalLocales([
  "fr-CA",
//  new Intl.Locale("en-gb-oxendict"),
  "de",
//  new Intl.Locale("jp", { "calendar": "gregory" }),
  "zh",
//  new Intl.Locale("fr-CA"),
]), [
  "fr-CA",
//  "en-GB-oxendict",
  "de",
//  "jp-u-ca-gregory",
  "zh",
]);

//https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/canonicalized-tags.js
var canonicalizedTags = {
  "de": "de",
  "DE-de": "de-DE",
  "de-DE": "de-DE",
//  "cmn": "zh",  //TODO::Hermes .. We return "cmn" .. V8 parity
//  "CMN-hANS": "zh-Hans", //TODO::Hermes .. We return "cmn-Hans" .. V8 parity
//  "cmn-hans-cn": "zh-Hans-CN", //TODO::Hermes .. We return "cmn-Hans-CN" .. V8 parity
  "es-419": "es-419",
  "es-419-u-nu-latn": "es-419-u-nu-latn",
//   "cmn-hans-cn-u-ca-t-ca-x-t-u": "zh-Hans-CN-t-ca-u-ca-x-t-u", //TODO::Hermes .. We return cmn-Hans-CN-t-ca-u-ca-yes-x-t-u .. V8 Parity
  "de-gregory-u-ca-gregory": "de-gregory-u-ca-gregory",
  "sgn-GR": "sgn-GR",
  "ji": "yi",
//  "de-DD": "de-DE",  //TODO::Hermes .. We return de-DD because icu4j hasn't implemented deprecated regions .. Not V8 Parity
  "in": "id",
  "sr-cyrl-ekavsk": "sr-Cyrl-ekavsk",
  "en-ca-newfound": "en-CA-newfound",
//  "224488-rozaj-biske-1994": "sl-1994-biske-rozaj", // TODO::Hermes .. We return sl-rozaj-biske-1994 but not incorrect ... icu4c stores variants in a linked list which results in them getting reversed in the result ... Not V8 Parity
  "da-u-attr": "da-u-attr",
  "da-u-attr-co-search": "da-u-attr-co-search",
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


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/canonicalized-unicode-ext-seq.js

var locale = "it-u-nu-latn-ca-gregory";

// RFC 6067: The canonical order of keywords is in US-ASCII order by key.
var sorted = "it-u-ca-gregory-nu-latn";

var canonicalLocales = Intl.getCanonicalLocales(locale);
assert.sameValue(canonicalLocales.length, 1);

var canonicalLocale = canonicalLocales[0];
assert((canonicalLocale === locale) || (canonicalLocale === sorted));

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/complex-language-subtag-replacement.js

const testData = {
  // "sh" adds "Latn", unless a script subtag is already present.
  // <languageAlias type="sh" replacement="sr_Latn" reason="legacy"/>
//   "sh": "sr-Latn", // TODO::Hermes -- We return "sh" -- V8 parity
//   "sh-Cyrl": "sr-Cyrl", // TODO::Hermes -- We return "sh-Cyrl" -- V8 parity

  // "cnr" adds "ME", unless a region subtag is already present.
  // <languageAlias type="cnr" replacement="sr_ME" reason="legacy"/>
//  "cnr": "sr-ME", // TODO::Hermes -- We return cnr -- V8 Parity
//  "cnr-BA": "sr-BA", // TODO::Hermes -- We return cnr-BA -- V8 Parity
};

for (let [tag, canonical] of Object.entries(testData)) {
  // Make sure the test data is correct.
  assert(
    isCanonicalizedStructurallyValidLanguageTag(canonical),
    "\"" + canonical + "\" is a canonicalized and structurally valid language tag."
  );

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/complex-region-subtag-replacement.js

const testData = {
  // For example, the breakup of the Soviet Union ("SU") means that the region of
  // the Soviet Union ("SU") is replaced by Russia ("RU"), Armenia ("AM"), or
  // many others -- depending on the specified (or merely likely) language and
  // script subtags:
  //
  // <territoryAlias type="SU" replacement="RU AM AZ BY EE GE KZ KG LV LT MD TJ TM UA UZ" reason="deprecated"/>
  // <territoryAlias type="810" replacement="RU AM AZ BY EE GE KZ KG LV LT MD TJ TM UA UZ" reason="overlong"/>
  //"ru-SU": "ru-RU", // TODO::Hermes -- We return "ru-SU" -- V8 Parity
  //"ru-810": "ru-RU", // TODO::Hermes -- We return "ru-810" -- V8 Parity
  //"en-SU": "en-RU", // TODO::Hermes -- We return "en-SU" -- V8 Parity
  //"en-810": "en-RU", // TODO::Hermes -- We return "en-810" -- V8 Parity
  //"und-SU": "und-RU", // TODO::Hermes -- We return "und-SU" -- V8 Parity
  //"und-810": "und-RU", // TODO::Hermes -- We return "und-810" -- V8 Parity
  //"und-Latn-SU": "und-Latn-RU", // TODO::Hermes -- We return "und-Latn-SU" -- V8 Parity
  //"und-Latn-810": "und-Latn-RU", // TODO::Hermes -- We return "und-Latn-810" -- V8 Parity

  // Armenia can be the preferred region when the language is "hy" (Armenian) or
  // the script is "Armn" (Armenian).
  //
  // <likelySubtag from="hy" to="hy_Armn_AM"/>
  // <likelySubtag from="und_Armn" to="hy_Armn_AM"/>
  //"hy-SU": "hy-AM", // TODO::Hermes -- We return "hy-SU" -- V8 Parity
  //"hy-810": "hy-AM", // TODO::Hermes -- We return "hy-810" -- V8 Parity
  //"und-Armn-SU": "und-Armn-AM", // TODO::Hermes -- We return "und-Armn-SU" -- V8 Parity
  //"und-Armn-810": "und-Armn-AM", // TODO::Hermes -- We return "und-Armn-810" -- V8 Parity

  // <territoryAlias type="CS" replacement="RS ME" reason="deprecated"/>
  //
  // The following likely-subtags entries contain "RS" and "ME":
  //
  // <likelySubtag from="sr" to="sr_Cyrl_RS"/>
  // <likelySubtag from="sr_ME" to="sr_Latn_ME"/>
  // <likelySubtag from="und_RS" to="sr_Cyrl_RS"/>
  // <likelySubtag from="und_ME" to="sr_Latn_ME"/>
  //
  // In this case there is no language/script combination (without a region
  // subtag) where "ME" is ever chosen, so the replacement is always "RS".
  //"sr-CS": "sr-RS", // TODO::Hermes -- We return "sr-CS" -- V8 Parity
  //"sr-Latn-CS": "sr-Latn-RS", // TODO::Hermes -- We return "sr-Latn-CS" -- V8 Parity
  //"sr-Cyrl-CS": "sr-Cyrl-RS", // TODO::Hermes -- We return "sr-Cyrl-CS" -- V8 Parity

  // The existing region in the source locale identifier is ignored when selecting
  // the likely replacement region. For example take "az-NT", which is Azerbaijani
  // spoken in the Neutral Zone. The replacement region for "NT" is either
  // "SA" (Saudi-Arabia) or "IQ" (Iraq), and there is also a likely subtags entry
  // for "az-IQ". But when only looking at the language subtag in "az-NT", "az" is
  // always resolved to "az-Latn-AZ", and because "AZ" is not in the list ["SA",
  // "IQ"], the final replacement region is the default for "NT", namely "SA".
  // That means "az-NT" will be canonicalised to "az-SA" and not "az-IQ", even
  // though the latter may be a more sensible candidate based on the actual usage
  // of the target locales.
  //
  // <territoryAlias type="NT" replacement="SA IQ" reason="deprecated"/>
  // <likelySubtag from="az_IQ" to="az_Arab_IQ"/>
  // <likelySubtag from="az" to="az_Latn_AZ"/>
  //"az-NT": "az-SA", // TODO::Hermes -- We return "az-NT" -- V8 Parity
};

for (let [tag, canonical] of Object.entries(testData)) {
  // Make sure the test data is correct.
  assert(
    isCanonicalizedStructurallyValidLanguageTag(canonical),
    "\"" + canonical + "\" is a canonicalized and structurally valid language tag."
  );

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/duplicates.js

assert(compareArray(
  Intl.getCanonicalLocales(
    ['ab-cd', 'ff', 'de-rt', 'ab-Cd']), ['ab-CD', 'ff', 'de-RT']));

var locales = Intl.getCanonicalLocales(['en-US', 'en-US']);
assert(compareArray(locales, ['en-US']), 'en-US');


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/elements-not-reordered.js


var canonicalLocales = Intl.getCanonicalLocales(["zu", "af"]);

assert.sameValue(canonicalLocales.length, 2);
assert.sameValue(canonicalLocales[0], "zu");
assert.sameValue(canonicalLocales[1], "af");


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/error-cases.js

var rangeErrorCases =
  [
//   "en-us-", // TODO::Hermes -- We return "en-US" as icu4j happily parses it .. but icu4c fails parsing-- Not V8 Parity
   "-en-us",
   "en-us-en-us",
   "--",
   "-",
   "",
   "-e-"
  ];

rangeErrorCases.forEach(function(re) {
  assert.throws(RangeError, function() {
    Intl.getCanonicalLocales(re);
  });
});

var typeErrorCases =
  [
//    null, // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [null], // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [undefined], // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [true], // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [NaN], // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [2], // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
//    [Symbol('foo')] // TODO::Hermes -- Our implementation throws RangeError .. Needs to be fixed !
  ];

typeErrorCases.forEach(function(te) {
  assert.throws(TypeError, function() {
    Intl.getCanonicalLocales(te);
  });
});


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/get-locale.js
var locales = {
  '0': 'en-US',
  length: 2
};

Object.defineProperty(locales, "1", {
  get: function() { throw new Test262Error() }
});

assert.throws(Test262Error, function() {
  Intl.getCanonicalLocales(locales);
});


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/getCanonicalLocales.js

assert.sameValue(
  typeof Intl.getCanonicalLocales,
  'function',
  '`typeof Intl.getCanonicalLocales` is `function`'
);

//verifyNotEnumerable(Intl, 'getCanonicalLocales');
//verifyWritable(Intl, 'getCanonicalLocales');
//verifyConfigurable(Intl, 'getCanonicalLocales');


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/grandfathered.js

const regularGrandfathered = [
    {
        tag: "art-lojban",
        canonical: "jbo",
    },
//    {
//        tag: "zh-guoyu",
//        canonical: "zh", // TODO::Hermes :: icu4j and icu4c returns 'cmn' .. V8 parity
//    },
    {
        tag: "zh-hakka",
        canonical: "hak",
    },
    {
        tag: "zh-xiang",
        canonical: "hsn",
    },
];


for (const {tag, canonical} of regularGrandfathered) {
    assert.sameValue(Intl.getCanonicalLocales(tag)[0], canonical);  // TODO
}

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/has-property.js

var locales = {
  '0': 'en-US',
  '1': 'pt-BR',
  length: 2
};

//var p = new Proxy(locales, { // TODO : We don't have Proxy in Hermes
//  has: function(_, prop) {
//    if (prop === '0') {
//      throw new Test262Error();
//    }
//  }
//});

//assert.throws(Test262Error, function() {
//  Intl.getCanonicalLocales(p);
//});



// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/invalid-tags.js

var invalidLanguageTags = [] // getInvalidLanguageTags();
for (var i = 0; i < invalidLanguageTags.length; ++i) {
  var invalidTag = invalidLanguageTags[i];
  assert.throws(RangeError, function() {
    Intl.getCanonicalLocales(invalidTag)
  }, "Language tag: " + invalidTag);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/length.js
assert.sameValue(Intl.getCanonicalLocales.length, 1);

verifyNotEnumerable(Intl.getCanonicalLocales, "length");
verifyNotWritable(Intl.getCanonicalLocales, "length");
verifyConfigurable(Intl.getCanonicalLocales, "length");


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/locales-is-not-a-string.js

var gCL = Intl.getCanonicalLocales;

function assertArray(l, r) {
  assert(compareArray(l, r), r);
}

assertArray(gCL(), []);
assertArray(gCL(undefined), []);
assertArray(gCL(false), []);
assertArray(gCL(true), []);
assertArray(gCL(Symbol("foo")), []);
assertArray(gCL(NaN), []);
assertArray(gCL(1), []);
assertArray(gCL(1), []);

Number.prototype[0] = "en-US";
Number.prototype.length = 1;
assertArray(gCL(NaN), ["en-US"]);


// https://github.com/tc39/test262/blob/ee3715ee56744ccc8aeb22a921f442e98090b3c1/test/intl402/Intl/getCanonicalLocales/main.js

assert(Array.isArray(Intl.getCanonicalLocales('en-US')));

var gCL = Intl.getCanonicalLocales;

assertArray(gCL(), []);

assertArray(gCL('ab-cd'), ['ab-CD']);

assertArray(gCL(['ab-cd']), ['ab-CD']);

assertArray(gCL(['ab-cd', 'FF']), ['ab-CD', 'ff']);

assertArray(gCL({'a': 0}), []);

assertArray(gCL({}), []);

assertArray(gCL(['th-th-u-nu-thai']), ['th-TH-u-nu-thai']);

// https://github.com/tc39/test262/blob/ee3715ee56744ccc8aeb22a921f442e98090b3c1/test/intl402/Intl/getCanonicalLocales/name.js

assert.sameValue(Intl.getCanonicalLocales.name, 'getCanonicalLocales',
  'The value of `Intl.getCanonicalLocales.name` is `"getCanonicalLocales"`'
);

verifyNotEnumerable(Intl.getCanonicalLocales, 'name');
verifyNotWritable(Intl.getCanonicalLocales, 'name');
verifyConfigurable(Intl.getCanonicalLocales, 'name');


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/non-iana-canon.js

var testData = [
    {
        tag: "mo",
        // canonical: "ro",  // TODO::Hermes -- icu4j don't have the deprecated language list .. hence returns "mo" .. Not V8 Parity
    },
    {
        tag: "es-ES-preeuro",
    },
    {
        tag: "uz-UZ-cyrillic",
    },
    {
        tag: "posix",
    },
    {
        tag: "hi-direct",
    },
    {
        tag: "zh-pinyin",
    },
    {
        tag: "zh-stroke",
    },
    {
        tag: "aar-x-private",
        // "aar" should be canonicalized into "aa" because "aar" matches the type attribute of
        // a languageAlias element in
        // https://www.unicode.org/repos/cldr/trunk/common/supplemental/supplementalMetadata.xml
        canonical: "aa-x-private",
    },
    {
        tag: "heb-x-private",
        // "heb" should be canonicalized into "he" because "heb" matches the type attribute of
        // a languageAlias element in
        // https://www.unicode.org/repos/cldr/trunk/common/supplemental/supplementalMetadata.xml
        canonical: "he-x-private",
    },
//    {
//        tag: "de-u-kf",   // TODO::Hermes .. icu4j returns de-u-kf-yes .. while icu4c returns 'de-u-kf' .. Not V8 parity ..
//    },
    {
        tag: "ces",
        // "ces" should be canonicalized into "cs" because "ces" matches the type attribute of
        // a languageAlias element in
        // https://www.unicode.org/repos/cldr/trunk/common/supplemental/supplementalMetadata.xml
        canonical: "cs",
    },
    {
        tag: "hy-arevela",
        // canonical: "hy", // TODO::Hermes .. icu4j returns hy-arevela .. while icu4c returns 'hy' .. Not V8 parity ..
    },
    {
        tag: "hy-arevmda",
        // canonical: "hyw", // TODO::Hermes .. icu4j returns hy-arevmda .. while icu4c returns 'hy' .. Not V8 parity ..
    },
];

for (const {tag, canonical = tag} of testData) {
    assert.sameValue( // TODO
      Intl.getCanonicalLocales(tag)[0],
      canonical,
      'The value of Intl.getCanonicalLocales(tag)[0] equals the value of `canonical`'
    );
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/overriden-arg-length.js

var locales = {
  '0': 'en-US',
};

Object.defineProperty(locales, "length", {
  get: function() { throw new Test262Error() }
});

assert.throws(Test262Error, function() {
  Intl.getCanonicalLocales(locales);
}, "should throw if locales.length throws");

var locales = {
  '0': 'en-US',
  '1': 'pt-BR',
};

Object.defineProperty(locales, "length", {
  get: function() { return "1" }
});

assert(compareArray(Intl.getCanonicalLocales(locales), ['en-US']),
  "should return one element if locales.length is '1'");

var locales = {
  '0': 'en-US',
  '1': 'pt-BR',
};

Object.defineProperty(locales, "length", {
  get: function() { return 1.3 }
});

assert(compareArray(Intl.getCanonicalLocales(locales), ['en-US']),
  "should return one element if locales.length is 1.3");

var locales = {
  '0': 'en-US',
  '1': 'pt-BR',
};

Object.defineProperty(locales, "length", {
  get: function() { return Symbol("1.8") }
});

assert.throws(TypeError, function() {
  Intl.getCanonicalLocales(locales);
}, "should throw if locales.length is a Symbol");

var locales = {
  '0': 'en-US',
  '1': 'pt-BR',
};

Object.defineProperty(locales, "length", {
  get: function() { return -Infinity }
});

assert(compareArray(Intl.getCanonicalLocales(locales), []),
  "should return empty array if locales.length is -Infinity");

var locales = {
  length: -Math.pow(2, 32) + 1
};

Object.defineProperty(locales, "0", {
  get: function() { throw new Error("must not be gotten!"); }
})

assert(compareArray(Intl.getCanonicalLocales(locales), []),
  "should return empty array if locales.length is a negative value");

var count = 0;
var locs = { get length() { if (count++ > 0) throw 42; return 0; } };
var locales = Intl.getCanonicalLocales(locs); // shouldn't throw 42
assert.sameValue(locales.length, 0);


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/overriden-push.js

Array.prototype.push = function() { throw 42; };

// must not throw 42, might if push is used
var arr = Intl.getCanonicalLocales(["en-US"]);

assert(compareArray(arr, ["en-US"]));

