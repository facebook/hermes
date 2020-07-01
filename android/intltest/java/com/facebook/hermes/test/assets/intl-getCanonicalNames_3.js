
// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/to-string.js

var locales = {
  '0': { toString: function() { locales[1] = 'pt-BR'; return 'en-US'; }},
  length: 2
};

assert(compareArray(Intl.getCanonicalLocales(locales), [ "en-US", "pt-BR" ]));



const testData = {
  // Variant subtags are alphabetically ordered.
  // "sl-t-sl-rozaj-biske-1994": "sl-t-sl-1994-biske-rozaj",

  // tfield subtags are alphabetically ordered.
  // (Also tests subtag case normalisation.)
  // "DE-T-M0-DIN-K0-QWERTZ": "de-t-k0-qwertz-m0-din", // TODO::Hermes V8 parity

  // "true" tvalue subtags aren't removed.
  // (UTS 35 version 36, §3.2.1 claims otherwise, but tkey must be followed by
  // tvalue, so that's likely a spec bug in UTS 35.)
  "en-t-m0-true": "en-t-m0-true",

  // tlang subtags are canonicalised.
  // "en-t-iw": "en-t-he", // TODO::Hermes V8 parity

  // Deprecated tvalue subtags are replaced by their preferred value.
  // "und-Latn-t-und-hani-m0-names": "und-Latn-t-und-hani-m0-prprname", // TODO::Hermes V8 parity
};

for (let [tag, canonical] of Object.entries(testData)) {
  // Make sure the test data is correct.
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/transformed-ext-invalid.js

const invalid = [ // TODO::Hermes -- We currently don't throw any error because icu4j don't throw any error while canonicalizing them .. icu4c returns errors with some of these
  // empty
//  "en-t",
//  "en-t-a",
//  "en-t-x",
//  "en-t-0",
//
//  // incomplete
//  "en-t-",
//  "en-t-en-",
//  "en-t-0x-",
//
//  // tlang: unicode_language_subtag must be 2-3 or 5-8 characters and mustn't
//  // contain extlang subtags.
//  "en-t-root",
//  "en-t-abcdefghi",
//  "en-t-ar-aao",
//
//  // tlang: unicode_script_subtag must be 4 alphabetical characters, can't
//  // be repeated.
//  "en-t-en-lat0",
//  "en-t-en-latn-latn",
//
//  // tlang: unicode_region_subtag must either be 2 alpha characters or a three
//  // digit code.
//  "en-t-en-0",
//  "en-t-en-00",
//  "en-t-en-0x",
//  "en-t-en-x0",
//  "en-t-en-latn-0",
//  "en-t-en-latn-00",
//  "en-t-en-latn-xyz",
//
//  // tlang: unicode_variant_subtag is either 5-8 alphanum characters or 4
//  // characters starting with a digit.
//  "en-t-en-abcdefghi",
//  "en-t-en-latn-gb-ab",
//  "en-t-en-latn-gb-abc",
//   "en-t-en-latn-gb-abcd",
//  "en-t-en-latn-gb-abcdefghi",
//
//  // tkey must be followed by tvalue.
//  "en-t-d0",
//  "en-t-d0-m0",
//  "en-t-d0-x-private",
];

for (let tag of invalid) {
  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a structurally valid language tag.");

  assert.throws(RangeError, () => Intl.getCanonicalLocales(tag), `${tag}`);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/transformed-ext-valid.js

const valid = [
// tlang with unicode_language_subtag.
  "en-t-en",
//
//  // tlang with unicode_script_subtag.
  "en-t-en-latn",
//
//  // tlang with unicode_region_subtag.
  "en-t-en-ca",
//
//  // tlang with unicode_script_subtag and unicode_region_subtag.
  "en-t-en-latn-ca",
//
//  // tlang with unicode_variant_subtag.
  "en-t-en-emodeng",
//
//  // tlang with unicode_script_subtag and unicode_variant_subtag.
  "en-t-en-latn-emodeng",
//
//  // tlang with unicode_script_subtag and unicode_variant_subtag.
  "en-t-en-ca-emodeng",
//
//  // tlang with unicode_script_subtag, unicode_region_subtag, and unicode_variant_subtag.
  "en-t-en-latn-ca-emodeng",
//
//  // No tlang. (Must contain at least one tfield.)
  "en-t-d0-ascii",
];

const extraFields = [
  // No extra tfield
  "",
//
//  // tfield with a tvalue consisting of a single subtag.
  "-i0-handwrit",
//
//  // tfield with a tvalue consisting of two subtags.
  "-s0-accents-publish",
];

for (let tag of valid) {
  for (let extra of extraFields) {
    let actualTag = tag + extra;

    // Make sure the test data is correct.
    assert(isCanonicalizedStructurallyValidLanguageTag(actualTag),
           "\"" + actualTag + "\" is a canonical and structurally valid language tag.");

    let result = Intl.getCanonicalLocales(actualTag);
    assert.sameValue(result.length, 1);
    assert.sameValue(result[0], actualTag);
  }
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-calendar.js

const testData = {
  // <type name="ethioaa" [...] alias="ethiopic-amete-alem"/>
  "ethiopic-amete-alem": "ethioaa",

  // <type name="islamic-civil" [...] />
  // <type name="islamicc" [...] deprecated="true" preferred="islamic-civil" alias="islamic-civil"/>
  //
  // "name" and "alias" for "islamic-civil" don't quite match of what's spec'ed in UTS 35, §3.2.1.
  // Specifically following §3.2.1 to the letter means "islamicc" is the canonical value whereas
  // "islamic-civil" is an alias value. Assume the definitions in
  // https://unicode.org/reports/tr35/#Unicode_Locale_Extension_Data_Files overrule UTS 35, §3.2.1.
  "islamicc": "islamic-civil",
};

for (let [alias, name] of Object.entries(testData)) {
  let tag = "und-u-ca-" + alias;
  let canonical = "und-u-ca-" + name;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-col-strength.js

const testData = {
  // <type name="level1" [...] alias="primary"/>
  "primary": "level1",

  // "secondary" doesn't match |uvalue|, so we can skip it.
  // <type name="level2" [...] alias="secondary"/>
  // "secondary": "level2", // TODO:: Hermes -- Somehow icu4j returns und-u-ks-yes .. V8 throws RangeError

  // <type name="level3" [...] alias="tertiary"/>
  "tertiary": "level3",

  // Neither "quaternary" nor "quarternary" match |uvalue|, so we can skip them.
  // <type name="level4" [...] alias="quaternary quarternary"/>
  // "quaternary": "level4", // TODO:: Hermes -- Somehow icu4j returns und-u-ks-yes .. V8 throws RangeError
  // "quarternary": "level4", // TODO:: Hermes -- Somehow icu4j returns und-u-ks-yes .. V8 throws RangeError

  // "identical" doesn't match |uvalue|, so we can skip it.
  // <type name="identic" [...] alias="identical"/>
  // "identical": "identic", // TODO:: Hermes -- Somehow icu4j returns und-u-ks-yes .. V8 throws RangeError
};

for (let [alias, name] of Object.entries(testData)) {
  let tag = "und-u-ks-" + alias;
  let canonical = "und-u-ks-" + name;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-measurement-system.js

const testData = {
  // <type name="uksystem" [...] alias="imperial" since="28" />
  "imperial": "uksystem", // TODO
};

for (let [alias, name] of Object.entries(testData)) {
  let tag = "und-u-ms-" + alias;
  let canonical = "und-u-ms-" + name;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}



// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-region.js

const testData = {
  // <subdivisionAlias type="no23" replacement="no50" reason="deprecated"/>
  "no23": "no50",

  // <subdivisionAlias type="cn11" replacement="cnbj" reason="deprecated"/>
  "cn11": "cnbj",

  // <subdivisionAlias type="cz10a" replacement="cz110" reason="deprecated"/>
  "cz10a": "cz110",

  // <subdivisionAlias type="fra" replacement="frges" reason="deprecated"/>
  "fra": "frges",

  // <subdivisionAlias type="frg" replacement="frges" reason="deprecated"/>
  "frg": "frges",

  // <subdivisionAlias type="lud" replacement="lucl ludi lurd luvd luwi" reason="deprecated"/>
  "lud": "lucl",
};

for (let [alias, name] of Object.entries(testData)) {
  let tag = "und-u-rg-" + alias;
  let canonical = "und-u-rg-" + name;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  // assert.sameValue(result[0], canonical); // TODO:: Hermes -- icu4j as well as icu4c doesn't do this translation .. V8 Parity
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-subdivision.js

const testData = {
//  // <subdivisionAlias type="no23" replacement="no50" reason="deprecated"/>
  "no23": "no50",
//
//  // <subdivisionAlias type="cn11" replacement="cnbj" reason="deprecated"/>
  "cn11": "cnbj",
//
//  // <subdivisionAlias type="cz10a" replacement="cz110" reason="deprecated"/>
  "cz10a": "cz110",
//
//  // <subdivisionAlias type="fra" replacement="frges" reason="deprecated"/>
  "fra": "frges",
//
//  // <subdivisionAlias type="frg" replacement="frges" reason="deprecated"/>
  "frg": "frges",
//
//  // <subdivisionAlias type="lud" replacement="lucl ludi lurd luvd luwi" reason="deprecated"/>
  "lud": "lucl",
};

for (let [alias, name] of Object.entries(testData)) {
  // Subdivision codes should always have a matching region subtag. This
  // shouldn't actually matter for canonicalisation, but let's not push our
  // luck and instead keep the language tag 'valid' per UTS 35, §3.6.5.
  let region = name.substring(0, 2).toUpperCase();

  let tag = `und-${region}-u-sd-${alias}`;
  let canonical = `und-${region}-u-sd-${name}`;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  // assert.sameValue(result[0], canonical); // TODO:: Hermes -- icu4j as well as icu4c doesn't do this translation .. V8 Parity
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-timezone.js

const testData = {
  // Similar to the "ca" extension key, assume "preferred" holds the canonical
  // value and "name" the alias value.

  // <type name="cnckg" [...] deprecated="true" preferred="cnsha"/>
  "cnckg": "cnsha",
//
//  // NB: "Eire" matches the |uvalue| production.
//  // <type name="iedub" [...] alias="Europe/Dublin Eire"/>
  "eire": "iedub",
//
//  // NB: "EST" matches the |uvalue| production.
//  // <type name="utcw05" [...] alias="Etc/GMT+5 EST"/>
  "est": "utcw05",
//
//  // NB: "GMT0" matches the |uvalue| production.
//  // <type name="gmt" [...] alias="Etc/GMT Etc/GMT+0 Etc/GMT-0 Etc/GMT0 Etc/Greenwich GMT GMT+0 GMT-0 GMT0 Greenwich"/>
  "gmt0": "gmt",
//
//  // NB: "UCT" matches the |uvalue| production.
//  // <type name="utc" [...] alias="Etc/UTC Etc/UCT Etc/Universal Etc/Zulu UCT UTC Universal Zulu"/>
  "uct": "utc",
//
//  // NB: "Zulu" matches the |uvalue| production.
//  // <type name="utc" [...] alias="Etc/UTC Etc/UCT Etc/Universal Etc/Zulu UCT UTC Universal Zulu"/>
  "zulu": "utc",
};

for (let [alias, name] of Object.entries(testData)) {
  let tag = "und-u-tz-" + alias;
  let canonical = "und-u-tz-" + name;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], canonical);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-canonicalize-yes-to-true.js

const unicodeKeys = [
  // <key name="kb" [...] alias="colBackwards">
  //   <type name="true" [...] alias="yes"/>
  "kb",
//
//  // <key name="kc" [...] alias="colCaseLevel">
//  //   <type name="true" [...] alias="yes"/>
  "kc",
//
//  // <key name="kh" [...] alias="colBackwards">
//  //   <type name="true" [...] alias="yes"/>
  "kh",
//
//  // <key name="kh" [...] alias="colHiraganaQuaternary">
//  //   <type name="true" [...] alias="yes"/>
  "kk",
//
//  // <key name="kn" [...] alias="colNumeric">
//  //   <type name="true" [...] alias="yes"/>
  "kn",
];

for (let key of unicodeKeys) {
  let tag = `und-u-${key}-yes`;
  let canonical = `und-u-${key}`;

  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(tag), false,
                   "\"" + tag + "\" isn't a canonical language tag.");
  assert(isCanonicalizedStructurallyValidLanguageTag(canonical),
         "\"" + canonical + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  // assert.sameValue(result[0], canonical); // TODO::Hermes -- icu4j converts 'un-u-kb-yes' to 'un-u-kb-true' !! icu4c gives 'un-u-kb' .. Not V8 parity.
}

// Test some other Unicode extension keys which don't contain an alias entry to
// canonicalise "yes" to "true".
const otherUnicodeKeys = [
   "ka", "kf", "kr", "ks", "kv",
];

for (let key of otherUnicodeKeys) {
  let tag = `und-u-${key}-yes`;

  // Make sure the test data is correct.
  assert(isCanonicalizedStructurallyValidLanguageTag(tag),
         "\"" + tag + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(tag);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], tag);
}



// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/unicode-ext-key-with-digit.js

const invalidCases = [
  // "en-u-c0", // TODO:: Hermes .. Both icu4j and icu4c happily parses this into canonical form. V8 parity
  // "en-u-00", // TODO:: Hermes .. Both icu4j and icu4c happily parses this into canonical form. V8 parity
];

// The first character is allowed to be a digit.
const validCases = [
   // "en-u-0c",  // TODO::Hermes .. for 2char unicode extensions, icu4j appends '-yes' to canonical form. Some context here: https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4j/main/tests/core/src/com/ibm/icu/dev/test/util/LocaleBuilderTest.java#L89
   // icu4c doesn't add the '-yes' suffix. Not V8 parity.
];

for (let invalid of invalidCases) {
  // Make sure the test data is correct.
  assert.sameValue(isCanonicalizedStructurallyValidLanguageTag(invalid), false,
                   "\"" + invalid + "\" isn't a structurally valid language tag.");

  assert.throws(RangeError, () => Intl.getCanonicalLocales(invalid));
}

for (let valid of validCases) {
  // Make sure the test data is correct.
  assert(isCanonicalizedStructurallyValidLanguageTag(valid),
         "\"" + valid + "\" is a canonical and structurally valid language tag.");

  let result = Intl.getCanonicalLocales(valid);
  assert.sameValue(result.length, 1);
  assert.sameValue(result[0], valid);
}


// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/weird-cases.js
var weirdCases =
  [
   "en-x-u-foo",
   "en-a-bar-x-u-foo",
   "en-x-u-foo-a-bar",
   "en-a-bar-u-baz-x-u-foo",
  ];

weirdCases.forEach(function (weird) {
  assert(compareArray(Intl.getCanonicalLocales(weird), [weird]));
});

// https://github.com/tc39/test262/blob/master/test/intl402/Intl/getCanonicalLocales/descriptor.js

// Note that this test case must be run at the end. In order to verify that the property is configurable, we delete the property.
// NOTE:: ANY CALL TO 'Intl.getCanonicalLocales' WILL FAIL.

verifyProperty(Intl, 'getCanonicalLocales', {
  writable: true,
  enumerable: false,
  configurable: true,
});