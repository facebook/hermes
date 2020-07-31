// Copyright (C) 2011 2012 Norbert Lindenberg. All rights reserved.
// Copyright (C) 2012 2013 Mozilla Corporation. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
description: |
    This file contains shared functions for the tests in the conformance test
    suite for the ECMAScript Internationalization API.
author: Norbert Lindenberg
defines:
  - testWithIntlConstructors
  - taintDataProperty
  - taintMethod
  - taintProperties
  - taintArray
  - getLocaleSupportInfo
  - getInvalidLanguageTags
  - isCanonicalizedStructurallyValidLanguageTag
  - getInvalidLocaleArguments
  - testOption
  - testForUnwantedRegExpChanges
  - isValidNumberingSystem
  - testNumberFormat
  - getDateTimeComponents
  - getDateTimeComponentValues
  - isCanonicalizedStructurallyValidTimeZoneName
---*/
/**
 */


/**
 * @description Calls the provided function for every service constructor in
 * the Intl object.
 * @param {Function} f the function to call for each service constructor in
 *   the Intl object.
 *   @param {Function} Constructor the constructor object to test with.
 */
function testWithIntlConstructors(f) {
  var constructors = ["Collator", "NumberFormat", "DateTimeFormat"];

  // Optionally supported Intl constructors.
  // NB: Intl.Locale isn't an Intl service constructor!
  ["PluralRules", "RelativeTimeFormat", "ListFormat", "DisplayNames"].forEach(function(constructor) {
    if (typeof Intl[constructor] === "function") {
      constructors[constructors.length] = constructor;
    }
  });

  constructors.forEach(function (constructor) {
    var Constructor = Intl[constructor];
    try {
      f(Constructor);
    } catch (e) {
      e.message += " (Testing with " + constructor + ".)";
      throw e;
    }
  });
}


/**
 * Taints a named data property of the given object by installing
 * a setter that throws an exception.
 * @param {object} obj the object whose data property to taint
 * @param {string} property the property to taint
 */
function taintDataProperty(obj, property) {
  Object.defineProperty(obj, property, {
    set: function(value) {
      $ERROR("Client code can adversely affect behavior: setter for " + property + ".");
    },
    enumerable: false,
    configurable: true
  });
}


/**
 * Taints a named method of the given object by replacing it with a function
 * that throws an exception.
 * @param {object} obj the object whose method to taint
 * @param {string} property the name of the method to taint
 */
function taintMethod(obj, property) {
  Object.defineProperty(obj, property, {
    value: function() {
      $ERROR("Client code can adversely affect behavior: method " + property + ".");
    },
    writable: true,
    enumerable: false,
    configurable: true
  });
}


/**
 * Taints the given properties (and similarly named properties) by installing
 * setters on Object.prototype that throw exceptions.
 * @param {Array} properties an array of property names to taint
 */
function taintProperties(properties) {
  properties.forEach(function (property) {
    var adaptedProperties = [property, "__" + property, "_" + property, property + "_", property + "__"];
    adaptedProperties.forEach(function (property) {
      taintDataProperty(Object.prototype, property);
    });
  });
}


/**
 * Taints the Array object by creating a setter for the property "0" and
 * replacing some key methods with functions that throw exceptions.
 */
function taintArray() {
  taintDataProperty(Array.prototype, "0");
  taintMethod(Array.prototype, "indexOf");
  taintMethod(Array.prototype, "join");
  taintMethod(Array.prototype, "push");
  taintMethod(Array.prototype, "slice");
  taintMethod(Array.prototype, "sort");
}


/**
 * Gets locale support info for the given constructor object, which must be one
 * of Intl constructors.
 * @param {object} Constructor the constructor for which to get locale support info
 * @return {object} locale support info with the following properties:
 *   supported: array of fully supported language tags
 *   byFallback: array of language tags that are supported through fallbacks
 *   unsupported: array of unsupported language tags
 */
function getLocaleSupportInfo(Constructor) {
  var languages = ["zh", "es", "en", "hi", "ur", "ar", "ja", "pa"];
  var scripts = ["Latn", "Hans", "Deva", "Arab", "Jpan", "Hant", "Guru"];
  var countries = ["CN", "IN", "US", "PK", "JP", "TW", "HK", "SG", "419"];

  var allTags = [];
  var i, j, k;
  var language, script, country;
  for (i = 0; i < languages.length; i++) {
    language = languages[i];
    allTags.push(language);
    for (j = 0; j < scripts.length; j++) {
      script = scripts[j];
      allTags.push(language + "-" + script);
      for (k = 0; k < countries.length; k++) {
        country = countries[k];
        allTags.push(language + "-" + script + "-" + country);
      }
    }
    for (k = 0; k < countries.length; k++) {
      country = countries[k];
      allTags.push(language + "-" + country);
    }
  }

  var supported = [];
  var byFallback = [];
  var unsupported = [];
  for (i = 0; i < allTags.length; i++) {
    var request = allTags[i];
    var result = new Constructor([request], {localeMatcher: "lookup"}).resolvedOptions().locale;
    if (request === result) {
      supported.push(request);
    } else if (request.indexOf(result) === 0) {
      byFallback.push(request);
    } else {
      unsupported.push(request);
    }
  }

  return {
    supported: supported,
    byFallback: byFallback,
    unsupported: unsupported
  };
}


/**
 * Returns an array of strings for which IsStructurallyValidLanguageTag() returns false
 */
function getInvalidLanguageTags() {
  var invalidLanguageTags = [
    "", // empty tag
    "i", // singleton alone
    "x", // private use without subtag
    "u", // extension singleton in first place
    "419", // region code in first place
    "u-nu-latn-cu-bob", // extension sequence without language
    "hans-cmn-cn", // "hans" could theoretically be a 4-letter language code,
                   // but those can't be followed by extlang codes.
    "cmn-hans-cn-u-u", // duplicate singleton
    "cmn-hans-cn-t-u-ca-u", // duplicate singleton
    "de-gregory-gregory", // duplicate variant
    "*", // language range
    "de-*", // language range
    "中文", // non-ASCII letters
    "en-ß", // non-ASCII letters
    "ıd", // non-ASCII letters
    "es-Latn-latn", // two scripts
    "pl-PL-pl", // two regions
    "u-ca-gregory", // extension in first place
    "de-1996-1996", // duplicate numeric variant
    "pt-u-ca-gregory-u-nu-latn", // duplicate singleton subtag

    // Invalid tags starting with: https://github.com/tc39/ecma402/pull/289
    "no-nyn", // regular grandfathered in BCP47, but invalid in UTS35
    "i-klingon", // irregular grandfathered in BCP47, but invalid in UTS35
    "zh-hak-CN", // language with extlang in BCP47, but invalid in UTS35
    "sgn-ils", // language with extlang in BCP47, but invalid in UTS35
    "x-foo", // privateuse-only in BCP47, but invalid in UTS35
    "x-en-US-12345", // more privateuse-only variants.
    "x-12345-12345-en-US",
    "x-en-US-12345-12345",
    "x-en-u-foo",
    "x-en-u-foo-u-bar",
    "x-u-foo",

    // underscores in different parts of the language tag
    "de_DE",
    "DE_de",
    "cmn_Hans",
    "cmn-hans_cn",
    "es_419",
    "es-419-u-nu-latn-cu_bob",
    "i_klingon",
    "cmn-hans-cn-t-ca-u-ca-x_t-u",
    "enochian_enochian",
    "de-gregory_u-ca-gregory",

    "en\u0000", // null-terminator sequence
    " en", // leading whitespace
    "en ", // trailing whitespace
    "it-IT-Latn", // country before script tag
    "de-u", // incomplete Unicode extension sequences
    "de-u-",
    "de-u-ca-",
    "de-u-ca-gregory-",
    "si-x", // incomplete private-use tags
    "x-",
    "x-y-",
  ];

  // make sure the data above is correct
  for (var i = 0; i < invalidLanguageTags.length; ++i) {
    var invalidTag = invalidLanguageTags[i];
    assert(
      !isCanonicalizedStructurallyValidLanguageTag(invalidTag),
      "Test data \"" + invalidTag + "\" is a canonicalized and structurally valid language tag."
    );
  }

  return invalidLanguageTags;
}


/**
 * @description Tests whether locale is a String value representing a
 * structurally valid and canonicalized BCP 47 language tag, as defined in
 * sections 6.2.2 and 6.2.3 of the ECMAScript Internationalization API
 * Specification.
 * @param {String} locale the string to be tested.
 * @result {Boolean} whether the test succeeded.
 */
function isCanonicalizedStructurallyValidLanguageTag(locale) {

  /**
   * Regular expression defining Unicode BCP 47 Locale Identifiers.
   *
   * Spec: https://unicode.org/reports/tr35/#Unicode_locale_identifier
   */
  var alpha = "[a-z]",
    digit = "[0-9]",
    alphanum = "[a-z0-9]",
    variant = "(" + alphanum + "{5,8}|(?:" + digit + alphanum + "{3}))",
    region = "(" + alpha + "{2}|" + digit + "{3})",
    script = "(" + alpha + "{4})",
    language = "(" + alpha + "{2,3}|" + alpha + "{5,8})",
    privateuse = "(x(-[a-z0-9]{1,8})+)",
    singleton = "(" + digit + "|[a-wy-z])",
    attribute= "(" + alphanum + "{3,8})",
    keyword = "(" + alphanum + alpha + "(-" + alphanum + "{3,8})*)",
    unicode_locale_extensions = "(u((-" + keyword + ")+|((-" + attribute + ")+(-" + keyword + ")*)))",
    tlang = "(" + language + "(-" + script + ")?(-" + region + ")?(-" + variant + ")*)",
    tfield = "(" + alpha + digit + "(-" + alphanum + "{3,8})+)",
    transformed_extensions = "(t((-" + tlang + "(-" + tfield + ")*)|(-" + tfield + ")+))",
    other_singleton = "(" + digit + "|[a-sv-wy-z])",
    other_extensions = "(" + other_singleton + "(-" + alphanum + "{2,8})+)",
    extension = "(" + unicode_locale_extensions + "|" + transformed_extensions + "|" + other_extensions + ")",
    locale_id = language + "(-" + script + ")?(-" + region + ")?(-" + variant + ")*(-" + extension + ")*(-" + privateuse + ")?",
    languageTag = "^(" + locale_id + ")$",
    languageTagRE = new RegExp(languageTag, "i");

  var duplicateSingleton = "-" + singleton + "-(.*-)?\\1(?!" + alphanum + ")",
    duplicateSingletonRE = new RegExp(duplicateSingleton, "i"),
    duplicateVariant = "(" + alphanum + "{2,8}-)+" + variant + "-(" + alphanum + "{2,8}-)*\\2(?!" + alphanum + ")",
    duplicateVariantRE = new RegExp(duplicateVariant, "i");

  var transformKeyRE = new RegExp("^" + alpha + digit + "$", "i");

  /**
   * Verifies that the given string is a well-formed Unicode BCP 47 Locale Identifier
   * with no duplicate variant or singleton subtags.
   *
   * Spec: ECMAScript Internationalization API Specification, draft, 6.2.2.
   */
  function isStructurallyValidLanguageTag(locale) {
    if (!languageTagRE.test(locale)) {
      return false;
    }
    locale = locale.split(/-x-/)[0];
    return !duplicateSingletonRE.test(locale) && !duplicateVariantRE.test(locale);
  }


  /**
   * Mappings from complete tags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __tagMappings = {
    // property names must be in lower case; values in canonical form

    "art-lojban": "jbo",
    "cel-gaulish": "xtg-x-cel-gaulish",
    "zh-guoyu": "zh",
    "zh-hakka": "hak",
    "zh-xiang": "hsn",
  };


  /**
   * Mappings from language subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __languageMappings = {
    // property names and values must be in canonical case

    "aam": "aas",
    "aar": "aa",
    "abk": "ab",
    "adp": "dz",
    "afr": "af",
    "aju": "jrb",
    "aka": "ak",
    "alb": "sq",
    "als": "sq",
    "amh": "am",
    "ara": "ar",
    "arb": "ar",
    "arg": "an",
    "arm": "hy",
    "asd": "snz",
    "asm": "as",
    "aue": "ktz",
    "ava": "av",
    "ave": "ae",
    "aym": "ay",
    "ayr": "ay",
    "ayx": "nun",
    "aze": "az",
    "azj": "az",
    "bak": "ba",
    "bam": "bm",
    "baq": "eu",
    "bcc": "bal",
    "bcl": "bik",
    "bel": "be",
    "ben": "bn",
    "bgm": "bcg",
    "bh": "bho",
    "bih": "bho",
    "bis": "bi",
    "bjd": "drl",
    "bod": "bo",
    "bos": "bs",
    "bre": "br",
    "bul": "bg",
    "bur": "my",
    "bxk": "luy",
    "bxr": "bua",
    "cat": "ca",
    "ccq": "rki",
    "ces": "cs",
    "cha": "ch",
    "che": "ce",
    "chi": "zh",
    "chu": "cu",
    "chv": "cv",
    "cjr": "mom",
    "cka": "cmr",
    "cld": "syr",
    "cmk": "xch",
    "cmn": "zh",
    "cor": "kw",
    "cos": "co",
    "coy": "pij",
    "cqu": "quh",
    "cre": "cr",
    "cwd": "cr",
    "cym": "cy",
    "cze": "cs",
    "dan": "da",
    "deu": "de",
    "dgo": "doi",
    "dhd": "mwr",
    "dik": "din",
    "diq": "zza",
    "dit": "dif",
    "div": "dv",
    "drh": "mn",
    "dut": "nl",
    "dzo": "dz",
    "ekk": "et",
    "ell": "el",
    "emk": "man",
    "eng": "en",
    "epo": "eo",
    "esk": "ik",
    "est": "et",
    "eus": "eu",
    "ewe": "ee",
    "fao": "fo",
    "fas": "fa",
    "fat": "ak",
    "fij": "fj",
    "fin": "fi",
    "fra": "fr",
    "fre": "fr",
    "fry": "fy",
    "fuc": "ff",
    "ful": "ff",
    "gav": "dev",
    "gaz": "om",
    "gbo": "grb",
    "geo": "ka",
    "ger": "de",
    "gfx": "vaj",
    "ggn": "gvr",
    "gla": "gd",
    "gle": "ga",
    "glg": "gl",
    "glv": "gv",
    "gno": "gon",
    "gre": "el",
    "grn": "gn",
    "gti": "nyc",
    "gug": "gn",
    "guj": "gu",
    "guv": "duz",
    "gya": "gba",
    "hat": "ht",
    "hau": "ha",
    "hdn": "hai",
    "hea": "hmn",
    "heb": "he",
    "her": "hz",
    "him": "srx",
    "hin": "hi",
    "hmo": "ho",
    "hrr": "jal",
    "hrv": "hr",
    "hun": "hu",
    "hye": "hy",
    "ibi": "opa",
    "ibo": "ig",
    "ice": "is",
    "ido": "io",
    "iii": "ii",
    "ike": "iu",
    "iku": "iu",
    "ile": "ie",
    "ilw": "gal",
    "in": "id",
    "ina": "ia",
    "ind": "id",
    "ipk": "ik",
    "isl": "is",
    "ita": "it",
    "iw": "he",
    "jav": "jv",
    "jeg": "oyb",
    "ji": "yi",
    "jpn": "ja",
    "jw": "jv",
    "kal": "kl",
    "kan": "kn",
    "kas": "ks",
    "kat": "ka",
    "kau": "kr",
    "kaz": "kk",
    "kgc": "tdf",
    "kgh": "kml",
    "khk": "mn",
    "khm": "km",
    "kik": "ki",
    "kin": "rw",
    "kir": "ky",
    "kmr": "ku",
    "knc": "kr",
    "kng": "kg",
    "knn": "kok",
    "koj": "kwv",
    "kom": "kv",
    "kon": "kg",
    "kor": "ko",
    "kpv": "kv",
    "krm": "bmf",
    "ktr": "dtp",
    "kua": "kj",
    "kur": "ku",
    "kvs": "gdj",
    "kwq": "yam",
    "kxe": "tvd",
    "kzj": "dtp",
    "kzt": "dtp",
    "lao": "lo",
    "lat": "la",
    "lav": "lv",
    "lbk": "bnc",
    "lii": "raq",
    "lim": "li",
    "lin": "ln",
    "lit": "lt",
    "llo": "ngt",
    "lmm": "rmx",
    "ltz": "lb",
    "lub": "lu",
    "lug": "lg",
    "lvs": "lv",
    "mac": "mk",
    "mah": "mh",
    "mal": "ml",
    "mao": "mi",
    "mar": "mr",
    "may": "ms",
    "meg": "cir",
    "mhr": "chm",
    "mkd": "mk",
    "mlg": "mg",
    "mlt": "mt",
    "mnk": "man",
    "mo": "ro",
    "mol": "ro",
    "mon": "mn",
    "mri": "mi",
    "msa": "ms",
    "mst": "mry",
    "mup": "raj",
    "mwj": "vaj",
    "mya": "my",
    "myd": "aog",
    "myt": "mry",
    "nad": "xny",
    "nau": "na",
    "nav": "nv",
    "nbl": "nr",
    "ncp": "kdz",
    "nde": "nd",
    "ndo": "ng",
    "nep": "ne",
    "nld": "nl",
    "nno": "nn",
    "nns": "nbr",
    "nnx": "ngv",
    "no": "nb",
    "nob": "nb",
    "nor": "nb",
    "npi": "ne",
    "nts": "pij",
    "nya": "ny",
    "oci": "oc",
    "ojg": "oj",
    "oji": "oj",
    "ori": "or",
    "orm": "om",
    "ory": "or",
    "oss": "os",
    "oun": "vaj",
    "pan": "pa",
    "pbu": "ps",
    "pcr": "adx",
    "per": "fa",
    "pes": "fa",
    "pli": "pi",
    "plt": "mg",
    "pmc": "huw",
    "pmu": "phr",
    "pnb": "lah",
    "pol": "pl",
    "por": "pt",
    "ppa": "bfy",
    "ppr": "lcq",
    "pry": "prt",
    "pus": "ps",
    "puz": "pub",
    "que": "qu",
    "quz": "qu",
    "rmy": "rom",
    "roh": "rm",
    "ron": "ro",
    "rum": "ro",
    "run": "rn",
    "rus": "ru",
    "sag": "sg",
    "san": "sa",
    "sca": "hle",
    "scc": "sr",
    "scr": "hr",
    "sin": "si",
    "skk": "oyb",
    "slk": "sk",
    "slo": "sk",
    "slv": "sl",
    "sme": "se",
    "smo": "sm",
    "sna": "sn",
    "snd": "sd",
    "som": "so",
    "sot": "st",
    "spa": "es",
    "spy": "kln",
    "sqi": "sq",
    "src": "sc",
    "srd": "sc",
    "srp": "sr",
    "ssw": "ss",
    "sun": "su",
    "swa": "sw",
    "swe": "sv",
    "swh": "sw",
    "tah": "ty",
    "tam": "ta",
    "tat": "tt",
    "tdu": "dtp",
    "tel": "te",
    "tgk": "tg",
    "tgl": "fil",
    "tha": "th",
    "thc": "tpo",
    "thx": "oyb",
    "tib": "bo",
    "tie": "ras",
    "tir": "ti",
    "tkk": "twm",
    "tl": "fil",
    "tlw": "weo",
    "tmp": "tyj",
    "tne": "kak",
    "ton": "to",
    "tsf": "taj",
    "tsn": "tn",
    "tso": "ts",
    "ttq": "tmh",
    "tuk": "tk",
    "tur": "tr",
    "tw": "ak",
    "twi": "ak",
    "uig": "ug",
    "ukr": "uk",
    "umu": "del",
    "uok": "ema",
    "urd": "ur",
    "uzb": "uz",
    "uzn": "uz",
    "ven": "ve",
    "vie": "vi",
    "vol": "vo",
    "wel": "cy",
    "wln": "wa",
    "wol": "wo",
    "xba": "cax",
    "xho": "xh",
    "xia": "acn",
    "xkh": "waw",
    "xpe": "kpe",
    "xsj": "suj",
    "xsl": "den",
    "ybd": "rki",
    "ydd": "yi",
    "yid": "yi",
    "yma": "lrr",
    "ymt": "mtm",
    "yor": "yo",
    "yos": "zom",
    "yuu": "yug",
    "zai": "zap",
    "zha": "za",
    "zho": "zh",
    "zsm": "ms",
    "zul": "zu",
    "zyb": "za",
  };


  /**
   * Mappings from region subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __regionMappings = {
    // property names and values must be in canonical case

    "004": "AF",
    "008": "AL",
    "010": "AQ",
    "012": "DZ",
    "016": "AS",
    "020": "AD",
    "024": "AO",
    "028": "AG",
    "031": "AZ",
    "032": "AR",
    "036": "AU",
    "040": "AT",
    "044": "BS",
    "048": "BH",
    "050": "BD",
    "051": "AM",
    "052": "BB",
    "056": "BE",
    "060": "BM",
    "062": "034",
    "064": "BT",
    "068": "BO",
    "070": "BA",
    "072": "BW",
    "074": "BV",
    "076": "BR",
    "084": "BZ",
    "086": "IO",
    "090": "SB",
    "092": "VG",
    "096": "BN",
    "100": "BG",
    "104": "MM",
    "108": "BI",
    "112": "BY",
    "116": "KH",
    "120": "CM",
    "124": "CA",
    "132": "CV",
    "136": "KY",
    "140": "CF",
    "144": "LK",
    "148": "TD",
    "152": "CL",
    "156": "CN",
    "158": "TW",
    "162": "CX",
    "166": "CC",
    "170": "CO",
    "174": "KM",
    "175": "YT",
    "178": "CG",
    "180": "CD",
    "184": "CK",
    "188": "CR",
    "191": "HR",
    "192": "CU",
    "196": "CY",
    "203": "CZ",
    "204": "BJ",
    "208": "DK",
    "212": "DM",
    "214": "DO",
    "218": "EC",
    "222": "SV",
    "226": "GQ",
    "230": "ET",
    "231": "ET",
    "232": "ER",
    "233": "EE",
    "234": "FO",
    "238": "FK",
    "239": "GS",
    "242": "FJ",
    "246": "FI",
    "248": "AX",
    "249": "FR",
    "250": "FR",
    "254": "GF",
    "258": "PF",
    "260": "TF",
    "262": "DJ",
    "266": "GA",
    "268": "GE",
    "270": "GM",
    "275": "PS",
    "276": "DE",
    "278": "DE",
    "280": "DE",
    "288": "GH",
    "292": "GI",
    "296": "KI",
    "300": "GR",
    "304": "GL",
    "308": "GD",
    "312": "GP",
    "316": "GU",
    "320": "GT",
    "324": "GN",
    "328": "GY",
    "332": "HT",
    "334": "HM",
    "336": "VA",
    "340": "HN",
    "344": "HK",
    "348": "HU",
    "352": "IS",
    "356": "IN",
    "360": "ID",
    "364": "IR",
    "368": "IQ",
    "372": "IE",
    "376": "IL",
    "380": "IT",
    "384": "CI",
    "388": "JM",
    "392": "JP",
    "398": "KZ",
    "400": "JO",
    "404": "KE",
    "408": "KP",
    "410": "KR",
    "414": "KW",
    "417": "KG",
    "418": "LA",
    "422": "LB",
    "426": "LS",
    "428": "LV",
    "430": "LR",
    "434": "LY",
    "438": "LI",
    "440": "LT",
    "442": "LU",
    "446": "MO",
    "450": "MG",
    "454": "MW",
    "458": "MY",
    "462": "MV",
    "466": "ML",
    "470": "MT",
    "474": "MQ",
    "478": "MR",
    "480": "MU",
    "484": "MX",
    "492": "MC",
    "496": "MN",
    "498": "MD",
    "499": "ME",
    "500": "MS",
    "504": "MA",
    "508": "MZ",
    "512": "OM",
    "516": "NA",
    "520": "NR",
    "524": "NP",
    "528": "NL",
    "531": "CW",
    "533": "AW",
    "534": "SX",
    "535": "BQ",
    "540": "NC",
    "548": "VU",
    "554": "NZ",
    "558": "NI",
    "562": "NE",
    "566": "NG",
    "570": "NU",
    "574": "NF",
    "578": "NO",
    "580": "MP",
    "581": "UM",
    "583": "FM",
    "584": "MH",
    "585": "PW",
    "586": "PK",
    "591": "PA",
    "598": "PG",
    "600": "PY",
    "604": "PE",
    "608": "PH",
    "612": "PN",
    "616": "PL",
    "620": "PT",
    "624": "GW",
    "626": "TL",
    "630": "PR",
    "634": "QA",
    "638": "RE",
    "642": "RO",
    "643": "RU",
    "646": "RW",
    "652": "BL",
    "654": "SH",
    "659": "KN",
    "660": "AI",
    "662": "LC",
    "663": "MF",
    "666": "PM",
    "670": "VC",
    "674": "SM",
    "678": "ST",
    "682": "SA",
    "686": "SN",
    "688": "RS",
    "690": "SC",
    "694": "SL",
    "702": "SG",
    "703": "SK",
    "704": "VN",
    "705": "SI",
    "706": "SO",
    "710": "ZA",
    "716": "ZW",
    "720": "YE",
    "724": "ES",
    "728": "SS",
    "729": "SD",
    "732": "EH",
    "736": "SD",
    "740": "SR",
    "744": "SJ",
    "748": "SZ",
    "752": "SE",
    "756": "CH",
    "760": "SY",
    "762": "TJ",
    "764": "TH",
    "768": "TG",
    "772": "TK",
    "776": "TO",
    "780": "TT",
    "784": "AE",
    "788": "TN",
    "792": "TR",
    "795": "TM",
    "796": "TC",
    "798": "TV",
    "800": "UG",
    "804": "UA",
    "807": "MK",
    "818": "EG",
    "826": "GB",
    "830": "JE",
    "831": "GG",
    "832": "JE",
    "833": "IM",
    "834": "TZ",
    "840": "US",
    "850": "VI",
    "854": "BF",
    "858": "UY",
    "860": "UZ",
    "862": "VE",
    "876": "WF",
    "882": "WS",
    "886": "YE",
    "887": "YE",
    "891": "RS",
    "894": "ZM",
    "958": "AA",
    "959": "QM",
    "960": "QN",
    "962": "QP",
    "963": "QQ",
    "964": "QR",
    "965": "QS",
    "966": "QT",
    "967": "EU",
    "968": "QV",
    "969": "QW",
    "970": "QX",
    "971": "QY",
    "972": "QZ",
    "973": "XA",
    "974": "XB",
    "975": "XC",
    "976": "XD",
    "977": "XE",
    "978": "XF",
    "979": "XG",
    "980": "XH",
    "981": "XI",
    "982": "XJ",
    "983": "XK",
    "984": "XL",
    "985": "XM",
    "986": "XN",
    "987": "XO",
    "988": "XP",
    "989": "XQ",
    "990": "XR",
    "991": "XS",
    "992": "XT",
    "993": "XU",
    "994": "XV",
    "995": "XW",
    "996": "XX",
    "997": "XY",
    "998": "XZ",
    "999": "ZZ",
    "BU": "MM",
    "CS": "RS",
    "CT": "KI",
    "DD": "DE",
    "DY": "BJ",
    "FQ": "AQ",
    "FX": "FR",
    "HV": "BF",
    "JT": "UM",
    "MI": "UM",
    "NH": "VU",
    "NQ": "AQ",
    "PU": "UM",
    "PZ": "PA",
    "QU": "EU",
    "RH": "ZW",
    "TP": "TL",
    "UK": "GB",
    "VD": "VN",
    "WK": "UM",
    "YD": "YE",
    "YU": "RS",
    "ZR": "CD",
  };


  /**
   * Complex mappings from language subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __complexLanguageMappings = {
    // property names and values must be in canonical case

    "cnr": {language: "sr", region: "ME"},
    "drw": {language: "fa", region: "AF"},
    "hbs": {language: "sr", script: "Latn"},
    "prs": {language: "fa", region: "AF"},
    "sh": {language: "sr", script: "Latn"},
    "swc": {language: "sw", region: "CD"},
    "tnf": {language: "fa", region: "AF"},
  };


  /**
   * Complex mappings from region subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __complexRegionMappings = {
    // property names and values must be in canonical case

    "172": {
      default: "RU",
      "ab": "GE",
      "az": "AZ",
      "be": "BY",
      "crh": "UA",
      "gag": "MD",
      "got": "UA",
      "hy": "AM",
      "ji": "UA",
      "ka": "GE",
      "kaa": "UZ",
      "kk": "KZ",
      "ku-Yezi": "GE",
      "ky": "KG",
      "os": "GE",
      "rue": "UA",
      "sog": "UZ",
      "tg": "TJ",
      "tk": "TM",
      "tkr": "AZ",
      "tly": "AZ",
      "ttt": "AZ",
      "ug-Cyrl": "KZ",
      "uk": "UA",
      "und-Armn": "AM",
      "und-Chrs": "UZ",
      "und-Geor": "GE",
      "und-Goth": "UA",
      "und-Sogd": "UZ",
      "und-Sogo": "UZ",
      "und-Yezi": "GE",
      "uz": "UZ",
      "xco": "UZ",
      "xmf": "GE",
    },
    "200": {
      default: "CZ",
      "sk": "SK",
    },
    "530": {
      default: "CW",
      "vic": "SX",
    },
    "532": {
      default: "CW",
      "vic": "SX",
    },
    "536": {
      default: "SA",
      "akk": "IQ",
      "ckb": "IQ",
      "ku-Arab": "IQ",
      "mis": "IQ",
      "syr": "IQ",
      "und-Hatr": "IQ",
      "und-Syrc": "IQ",
      "und-Xsux": "IQ",
    },
    "582": {
      default: "FM",
      "mh": "MH",
      "pau": "PW",
    },
    "810": {
      default: "RU",
      "ab": "GE",
      "az": "AZ",
      "be": "BY",
      "crh": "UA",
      "et": "EE",
      "gag": "MD",
      "got": "UA",
      "hy": "AM",
      "ji": "UA",
      "ka": "GE",
      "kaa": "UZ",
      "kk": "KZ",
      "ku-Yezi": "GE",
      "ky": "KG",
      "lt": "LT",
      "ltg": "LV",
      "lv": "LV",
      "os": "GE",
      "rue": "UA",
      "sgs": "LT",
      "sog": "UZ",
      "tg": "TJ",
      "tk": "TM",
      "tkr": "AZ",
      "tly": "AZ",
      "ttt": "AZ",
      "ug-Cyrl": "KZ",
      "uk": "UA",
      "und-Armn": "AM",
      "und-Chrs": "UZ",
      "und-Geor": "GE",
      "und-Goth": "UA",
      "und-Sogd": "UZ",
      "und-Sogo": "UZ",
      "und-Yezi": "GE",
      "uz": "UZ",
      "vro": "EE",
      "xco": "UZ",
      "xmf": "GE",
    },
    "890": {
      default: "RS",
      "bs": "BA",
      "hr": "HR",
      "mk": "MK",
      "sl": "SI",
    },
    "AN": {
      default: "CW",
      "vic": "SX",
    },
    "NT": {
      default: "SA",
      "akk": "IQ",
      "ckb": "IQ",
      "ku-Arab": "IQ",
      "mis": "IQ",
      "syr": "IQ",
      "und-Hatr": "IQ",
      "und-Syrc": "IQ",
      "und-Xsux": "IQ",
    },
    "PC": {
      default: "FM",
      "mh": "MH",
      "pau": "PW",
    },
    "SU": {
      default: "RU",
      "ab": "GE",
      "az": "AZ",
      "be": "BY",
      "crh": "UA",
      "et": "EE",
      "gag": "MD",
      "got": "UA",
      "hy": "AM",
      "ji": "UA",
      "ka": "GE",
      "kaa": "UZ",
      "kk": "KZ",
      "ku-Yezi": "GE",
      "ky": "KG",
      "lt": "LT",
      "ltg": "LV",
      "lv": "LV",
      "os": "GE",
      "rue": "UA",
      "sgs": "LT",
      "sog": "UZ",
      "tg": "TJ",
      "tk": "TM",
      "tkr": "AZ",
      "tly": "AZ",
      "ttt": "AZ",
      "ug-Cyrl": "KZ",
      "uk": "UA",
      "und-Armn": "AM",
      "und-Chrs": "UZ",
      "und-Geor": "GE",
      "und-Goth": "UA",
      "und-Sogd": "UZ",
      "und-Sogo": "UZ",
      "und-Yezi": "GE",
      "uz": "UZ",
      "vro": "EE",
      "xco": "UZ",
      "xmf": "GE",
    },
  };


  /**
   * Mappings from variant subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __variantMappings = {
    // property names and values must be in canonical case

    "aaland": {type: "region", replacement: "AX"},
    "arevela": {type: "language", replacement: "hy"},
    "arevmda": {type: "language", replacement: "hyw"},
    "heploc": {type: "variant", replacement: "alalc97"},
    "polytoni": {type: "variant", replacement: "polyton"},
  };


  /**
   * Mappings from Unicode extension subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __unicodeMappings = {
    // property names and values must be in canonical case

    "ca": {
      "ethiopic-amete-alem": "ethioaa",
      "islamicc": "islamic-civil",
    },
    "kb": {
      "yes": "true",
    },
    "kc": {
      "yes": "true",
    },
    "kh": {
      "yes": "true",
    },
    "kk": {
      "yes": "true",
    },
    "kn": {
      "yes": "true",
    },
    "ks": {
      "primary": "level1",
      "tertiary": "level3",
    },
    "ms": {
      "imperial": "uksystem",
    },
    "rg": {
      "cn11": "cnbj",
      "cn12": "cntj",
      "cn13": "cnhe",
      "cn14": "cnsx",
      "cn15": "cnmn",
      "cn21": "cnln",
      "cn22": "cnjl",
      "cn23": "cnhl",
      "cn31": "cnsh",
      "cn32": "cnjs",
      "cn33": "cnzj",
      "cn34": "cnah",
      "cn35": "cnfj",
      "cn36": "cnjx",
      "cn37": "cnsd",
      "cn41": "cnha",
      "cn42": "cnhb",
      "cn43": "cnhn",
      "cn44": "cngd",
      "cn45": "cngx",
      "cn46": "cnhi",
      "cn50": "cncq",
      "cn51": "cnsc",
      "cn52": "cngz",
      "cn53": "cnyn",
      "cn54": "cnxz",
      "cn61": "cnsn",
      "cn62": "cngs",
      "cn63": "cnqh",
      "cn64": "cnnx",
      "cn65": "cnxj",
      "cz10a": "cz110",
      "cz10b": "cz111",
      "cz10c": "cz112",
      "cz10d": "cz113",
      "cz10e": "cz114",
      "cz10f": "cz115",
      "cz611": "cz663",
      "cz612": "cz632",
      "cz613": "cz633",
      "cz614": "cz634",
      "cz615": "cz635",
      "cz621": "cz641",
      "cz622": "cz642",
      "cz623": "cz643",
      "cz624": "cz644",
      "cz626": "cz646",
      "cz627": "cz647",
      "czjc": "cz31",
      "czjm": "cz64",
      "czka": "cz41",
      "czkr": "cz52",
      "czli": "cz51",
      "czmo": "cz80",
      "czol": "cz71",
      "czpa": "cz53",
      "czpl": "cz32",
      "czpr": "cz10",
      "czst": "cz20",
      "czus": "cz42",
      "czvy": "cz63",
      "czzl": "cz72",
      "fra": "frges",
      "frb": "frnaq",
      "frc": "frara",
      "frd": "frbfc",
      "fre": "frbre",
      "frf": "frcvl",
      "frg": "frges",
      "frh": "frcor",
      "fri": "frbfc",
      "frj": "fridf",
      "frk": "frocc",
      "frl": "frnaq",
      "frm": "frges",
      "frn": "frocc",
      "fro": "frhdf",
      "frp": "frnor",
      "frq": "frnor",
      "frr": "frpdl",
      "frs": "frhdf",
      "frt": "frnaq",
      "fru": "frpac",
      "frv": "frara",
      "laxn": "laxs",
      "lud": "lucl",
      "lug": "luec",
      "lul": "luca",
      "mrnkc": "mr13",
      "no23": "no50",
      "nzn": "nzauk",
      "nzs": "nzcan",
      "omba": "ombj",
      "omsh": "omsj",
      "plds": "pl02",
      "plkp": "pl04",
      "pllb": "pl08",
      "plld": "pl10",
      "pllu": "pl06",
      "plma": "pl12",
      "plmz": "pl14",
      "plop": "pl16",
      "plpd": "pl20",
      "plpk": "pl18",
      "plpm": "pl22",
      "plsk": "pl26",
      "plsl": "pl24",
      "plwn": "pl28",
      "plwp": "pl30",
      "plzp": "pl32",
      "tteto": "tttob",
      "ttrcm": "ttmrc",
      "ttwto": "tttob",
      "twkhq": "twkhh",
      "twtnq": "twtnn",
      "twtpq": "twnwt",
      "twtxq": "twtxg",
    },
    "sd": {
      "cn11": "cnbj",
      "cn12": "cntj",
      "cn13": "cnhe",
      "cn14": "cnsx",
      "cn15": "cnmn",
      "cn21": "cnln",
      "cn22": "cnjl",
      "cn23": "cnhl",
      "cn31": "cnsh",
      "cn32": "cnjs",
      "cn33": "cnzj",
      "cn34": "cnah",
      "cn35": "cnfj",
      "cn36": "cnjx",
      "cn37": "cnsd",
      "cn41": "cnha",
      "cn42": "cnhb",
      "cn43": "cnhn",
      "cn44": "cngd",
      "cn45": "cngx",
      "cn46": "cnhi",
      "cn50": "cncq",
      "cn51": "cnsc",
      "cn52": "cngz",
      "cn53": "cnyn",
      "cn54": "cnxz",
      "cn61": "cnsn",
      "cn62": "cngs",
      "cn63": "cnqh",
      "cn64": "cnnx",
      "cn65": "cnxj",
      "cz10a": "cz110",
      "cz10b": "cz111",
      "cz10c": "cz112",
      "cz10d": "cz113",
      "cz10e": "cz114",
      "cz10f": "cz115",
      "cz611": "cz663",
      "cz612": "cz632",
      "cz613": "cz633",
      "cz614": "cz634",
      "cz615": "cz635",
      "cz621": "cz641",
      "cz622": "cz642",
      "cz623": "cz643",
      "cz624": "cz644",
      "cz626": "cz646",
      "cz627": "cz647",
      "czjc": "cz31",
      "czjm": "cz64",
      "czka": "cz41",
      "czkr": "cz52",
      "czli": "cz51",
      "czmo": "cz80",
      "czol": "cz71",
      "czpa": "cz53",
      "czpl": "cz32",
      "czpr": "cz10",
      "czst": "cz20",
      "czus": "cz42",
      "czvy": "cz63",
      "czzl": "cz72",
      "fra": "frges",
      "frb": "frnaq",
      "frc": "frara",
      "frd": "frbfc",
      "fre": "frbre",
      "frf": "frcvl",
      "frg": "frges",
      "frh": "frcor",
      "fri": "frbfc",
      "frj": "fridf",
      "frk": "frocc",
      "frl": "frnaq",
      "frm": "frges",
      "frn": "frocc",
      "fro": "frhdf",
      "frp": "frnor",
      "frq": "frnor",
      "frr": "frpdl",
      "frs": "frhdf",
      "frt": "frnaq",
      "fru": "frpac",
      "frv": "frara",
      "laxn": "laxs",
      "lud": "lucl",
      "lug": "luec",
      "lul": "luca",
      "mrnkc": "mr13",
      "no23": "no50",
      "nzn": "nzauk",
      "nzs": "nzcan",
      "omba": "ombj",
      "omsh": "omsj",
      "plds": "pl02",
      "plkp": "pl04",
      "pllb": "pl08",
      "plld": "pl10",
      "pllu": "pl06",
      "plma": "pl12",
      "plmz": "pl14",
      "plop": "pl16",
      "plpd": "pl20",
      "plpk": "pl18",
      "plpm": "pl22",
      "plsk": "pl26",
      "plsl": "pl24",
      "plwn": "pl28",
      "plwp": "pl30",
      "plzp": "pl32",
      "tteto": "tttob",
      "ttrcm": "ttmrc",
      "ttwto": "tttob",
      "twkhq": "twkhh",
      "twtnq": "twtnn",
      "twtpq": "twnwt",
      "twtxq": "twtxg",
    },
    "tz": {
      "aqams": "nzakl",
      "cnckg": "cnsha",
      "cnhrb": "cnsha",
      "cnkhg": "cnurc",
      "cuba": "cuhav",
      "egypt": "egcai",
      "eire": "iedub",
      "est": "utcw05",
      "gmt0": "gmt",
      "hongkong": "hkhkg",
      "hst": "utcw10",
      "iceland": "isrey",
      "iran": "irthr",
      "israel": "jeruslm",
      "jamaica": "jmkin",
      "japan": "jptyo",
      "libya": "lytip",
      "mst": "utcw07",
      "navajo": "usden",
      "poland": "plwaw",
      "portugal": "ptlis",
      "prc": "cnsha",
      "roc": "twtpe",
      "rok": "krsel",
      "turkey": "trist",
      "uct": "utc",
      "usnavajo": "usden",
      "zulu": "utc",
    },
  };


  /**
   * Mappings from Unicode extension subtags to preferred values.
   *
   * Spec: http://unicode.org/reports/tr35/#Identifiers
   * Version: CLDR, version 36.1
   */
  var __transformMappings = {
    // property names and values must be in canonical case

    "d0": {
      "name": "charname",
    },
    "m0": {
      "names": "prprname",
    },
  };

  /**
   * Canonicalizes the given well-formed BCP 47 language tag, including regularized case of subtags.
   *
   * Spec: ECMAScript Internationalization API Specification, draft, 6.2.3.
   * Spec: RFC 5646, section 4.5.
   */
  function canonicalizeLanguageTag(locale) {

    // start with lower case for easier processing, and because most subtags will need to be lower case anyway
    locale = locale.toLowerCase();

    // handle mappings for complete tags
    if (__tagMappings.hasOwnProperty(locale)) {
      return __tagMappings[locale];
    }

    var subtags = locale.split("-");
    var i = 0;

    // handle standard part: all subtags before first variant or singleton subtag
    var language;
    var script;
    var region;
    while (i < subtags.length) {
      var subtag = subtags[i];
      if (i === 0) {
        language = subtag;
      } else if (subtag.length === 2 || subtag.length === 3) {
        region = subtag.toUpperCase();
      } else if (subtag.length === 4 && !("0" <= subtag[0] && subtag[0] <= "9")) {
        script = subtag[0].toUpperCase() + subtag.substring(1).toLowerCase();
      } else {
        break;
      }
      i++;
    }

    if (__languageMappings.hasOwnProperty(language)) {
      language = __languageMappings[language];
    } else if (__complexLanguageMappings.hasOwnProperty(language)) {
      var mapping = __complexLanguageMappings[language];

      language = mapping.language;
      if (script === undefined && mapping.hasOwnProperty("script")) {
        script = mapping.script;
      }
      if (region === undefined && mapping.hasOwnProperty("region")) {
        region = mapping.region;
      }
    }

    if (region !== undefined) {
      if (__regionMappings.hasOwnProperty(region)) {
        region = __regionMappings[region];
      } else if (__complexRegionMappings.hasOwnProperty(region)) {
        var mapping = __complexRegionMappings[region];

        var mappingKey = language;
        if (script !== undefined) {
          mappingKey += "-" + script;
        }

        if (mapping.hasOwnProperty(mappingKey)) {
          region = mapping[mappingKey];
        } else {
          region = mapping.default;
        }
      }
    }

    // handle variants
    var variants = [];
    while (i < subtags.length && subtags[i].length > 1) {
      var variant = subtags[i];

      if (__variantMappings.hasOwnProperty(variant)) {
        var mapping = __variantMappings[variant];
        switch (mapping.type) {
          case "language":
            language = mapping.replacement;
            break;

          case "region":
            region = mapping.replacement;
            break;

          case "variant":
            variants.push(mapping.replacement);
            break;

          default:
            throw new Error("illegal variant mapping type");
        }
      } else {
        variants.push(variant);
      }

      i += 1;
    }
    variants.sort();

    // handle extensions
    var extensions = [];
    while (i < subtags.length && subtags[i] !== "x") {
      var extensionStart = i;
      i++;
      while (i < subtags.length && subtags[i].length > 1) {
        i++;
      }

      var extension;
      var extensionKey = subtags[extensionStart];
      if (extensionKey === "u") {
        var j = extensionStart + 1;

        // skip over leading attributes
        while (j < i && subtags[j].length > 2) {
          j++;
        }

        extension = subtags.slice(extensionStart, j).join("-");

        while (j < i) {
          var keyStart = j;
          j++;

          while (j < i && subtags[j].length > 2) {
            j++;
          }

          var key = subtags[keyStart];
          var value = subtags.slice(keyStart + 1, j).join("-");

          if (__unicodeMappings.hasOwnProperty(key)) {
            var mapping = __unicodeMappings[key];
            if (mapping.hasOwnProperty(value)) {
              value = mapping[value];
            }
          }

          extension += "-" + key;
          if (value !== "" && value !== "true") {
            extension += "-" + value;
          }
        }
      } else if (extensionKey === "t") {
        var j = extensionStart + 1;

        while (j < i && !transformKeyRE.test(subtags[j])) {
          j++;
        }

        extension = "t";

        var transformLanguage = subtags.slice(extensionStart + 1, j).join("-");
        if (transformLanguage !== "") {
          extension += "-" + canonicalizeLanguageTag(transformLanguage).toLowerCase();
        }

        while (j < i) {
          var keyStart = j;
          j++;

          while (j < i && subtags[j].length > 2) {
            j++;
          }

          var key = subtags[keyStart];
          var value = subtags.slice(keyStart + 1, j).join("-");

          if (__transformMappings.hasOwnProperty(key)) {
            var mapping = __transformMappings[key];
            if (mapping.hasOwnProperty(value)) {
              value = mapping[value];
            }
          }

          extension += "-" + key + "-" + value;
        }
      } else {
        extension = subtags.slice(extensionStart, i).join("-");
      }

      extensions.push(extension);
    }
    extensions.sort();

    // handle private use
    var privateUse;
    if (i < subtags.length) {
      privateUse = subtags.slice(i).join("-");
    }

    // put everything back together
    var canonical = language;
    if (script !== undefined) {
      canonical += "-" + script;
    }
    if (region !== undefined) {
      canonical += "-" + region;
    }
    if (variants.length > 0) {
      canonical += "-" + variants.join("-");
    }
    if (extensions.length > 0) {
      canonical += "-" + extensions.join("-");
    }
    if (privateUse !== undefined) {
      if (canonical.length > 0) {
        canonical += "-" + privateUse;
      } else {
        canonical = privateUse;
      }
    }

    return canonical;
  }

  return typeof locale === "string" && isStructurallyValidLanguageTag(locale) &&
      canonicalizeLanguageTag(locale) === locale;
}


/**
 * Returns an array of error cases handled by CanonicalizeLocaleList().
 */
function getInvalidLocaleArguments() {
  function CustomError() {}

  var topLevelErrors = [
    // fails ToObject
    [null, TypeError],

    // fails Get
    [{ get length() { throw new CustomError(); } }, CustomError],

    // fail ToLength
    [{ length: Symbol.toPrimitive }, TypeError],
    [{ length: { get [Symbol.toPrimitive]() { throw new CustomError(); } } }, CustomError],
    [{ length: { [Symbol.toPrimitive]() { throw new CustomError(); } } }, CustomError],
    [{ length: { get valueOf() { throw new CustomError(); } } }, CustomError],
    [{ length: { valueOf() { throw new CustomError(); } } }, CustomError],
    [{ length: { get toString() { throw new CustomError(); } } }, CustomError],
    [{ length: { toString() { throw new CustomError(); } } }, CustomError],

    // fail type check
    [[undefined], TypeError],
    [[null], TypeError],
    [[true], TypeError],
    [[Symbol.toPrimitive], TypeError],
    [[1], TypeError],
    [[0.1], TypeError],
    [[NaN], TypeError],
  ];

  var invalidLanguageTags = [
    "", // empty tag
    "i", // singleton alone
    "x", // private use without subtag
    "u", // extension singleton in first place
    "419", // region code in first place
    "u-nu-latn-cu-bob", // extension sequence without language
    "hans-cmn-cn", // "hans" could theoretically be a 4-letter language code,
                   // but those can't be followed by extlang codes.
    "abcdefghi", // overlong language
    "cmn-hans-cn-u-u", // duplicate singleton
    "cmn-hans-cn-t-u-ca-u", // duplicate singleton
    "de-gregory-gregory", // duplicate variant
    "*", // language range
    "de-*", // language range
    "中文", // non-ASCII letters
    "en-ß", // non-ASCII letters
    "ıd" // non-ASCII letters
  ];

  return topLevelErrors.concat(
    invalidLanguageTags.map(tag => [tag, RangeError]),
    invalidLanguageTags.map(tag => [[tag], RangeError]),
    invalidLanguageTags.map(tag => [["en", tag], RangeError]),
  )
}

/**
 * Tests whether the named options property is correctly handled by the given constructor.
 * @param {object} Constructor the constructor to test.
 * @param {string} property the name of the options property to test.
 * @param {string} type the type that values of the property are expected to have
 * @param {Array} [values] an array of allowed values for the property. Not needed for boolean.
 * @param {any} fallback the fallback value that the property assumes if not provided.
 * @param {object} testOptions additional options:
 *   @param {boolean} isOptional whether support for this property is optional for implementations.
 *   @param {boolean} noReturn whether the resulting value of the property is not returned.
 *   @param {boolean} isILD whether the resulting value of the property is implementation and locale dependent.
 *   @param {object} extra additional option to pass along, properties are value -> {option: value}.
 */
function testOption(Constructor, property, type, values, fallback, testOptions) {
  var isOptional = testOptions !== undefined && testOptions.isOptional === true;
  var noReturn = testOptions !== undefined && testOptions.noReturn === true;
  var isILD = testOptions !== undefined && testOptions.isILD === true;

  function addExtraOptions(options, value, testOptions) {
    if (testOptions !== undefined && testOptions.extra !== undefined) {
      var extra;
      if (value !== undefined && testOptions.extra[value] !== undefined) {
        extra = testOptions.extra[value];
      } else if (testOptions.extra.any !== undefined) {
        extra = testOptions.extra.any;
      }
      if (extra !== undefined) {
        Object.getOwnPropertyNames(extra).forEach(function (prop) {
          options[prop] = extra[prop];
        });
      }
    }
  }

  var testValues, options, obj, expected, actual, error;

  // test that the specified values are accepted. Also add values that convert to specified values.
  if (type === "boolean") {
    if (values === undefined) {
      values = [true, false];
    }
    testValues = values.slice(0);
    testValues.push(888);
    testValues.push(0);
  } else if (type === "string") {
    testValues = values.slice(0);
    testValues.push({toString: function () { return values[0]; }});
  }
  testValues.forEach(function (value) {
    options = {};
    options[property] = value;
    addExtraOptions(options, value, testOptions);
    obj = new Constructor(undefined, options);
    if (noReturn) {
      if (obj.resolvedOptions().hasOwnProperty(property)) {
        $ERROR("Option property " + property + " is returned, but shouldn't be.");
      }
    } else {
      actual = obj.resolvedOptions()[property];
      if (isILD) {
        if (actual !== undefined && values.indexOf(actual) === -1) {
          $ERROR("Invalid value " + actual + " returned for property " + property + ".");
        }
      } else {
        if (type === "boolean") {
          expected = Boolean(value);
        } else if (type === "string") {
          expected = String(value);
        }
        if (actual !== expected && !(isOptional && actual === undefined)) {
          $ERROR("Option value " + value + " for property " + property +
            " was not accepted; got " + actual + " instead.");
        }
      }
    }
  });

  // test that invalid values are rejected
  if (type === "string") {
    var invalidValues = ["invalidValue", -1, null];
    // assume that we won't have values in caseless scripts
    if (values[0].toUpperCase() !== values[0]) {
      invalidValues.push(values[0].toUpperCase());
    } else {
      invalidValues.push(values[0].toLowerCase());
    }
    invalidValues.forEach(function (value) {
      options = {};
      options[property] = value;
      addExtraOptions(options, value, testOptions);
      error = undefined;
      try {
        obj = new Constructor(undefined, options);
      } catch (e) {
        error = e;
      }
      if (error === undefined) {
        $ERROR("Invalid option value " + value + " for property " + property + " was not rejected.");
      } else if (error.name !== "RangeError") {
        $ERROR("Invalid option value " + value + " for property " + property + " was rejected with wrong error " + error.name + ".");
      }
    });
  }

  // test that fallback value or another valid value is used if no options value is provided
  if (!noReturn) {
    options = {};
    addExtraOptions(options, undefined, testOptions);
    obj = new Constructor(undefined, options);
    actual = obj.resolvedOptions()[property];
    if (!(isOptional && actual === undefined)) {
      if (fallback !== undefined) {
        if (actual !== fallback) {
          $ERROR("Option fallback value " + fallback + " for property " + property +
            " was not used; got " + actual + " instead.");
        }
      } else {
        if (values.indexOf(actual) === -1 && !(isILD && actual === undefined)) {
          $ERROR("Invalid value " + actual + " returned for property " + property + ".");
        }
      }
    }
  }
}


/**
 * Properties of the RegExp constructor that may be affected by use of regular
 * expressions, and the default values of these properties. Properties are from
 * https://developer.mozilla.org/en-US/docs/JavaScript/Reference/Deprecated_and_obsolete_features#RegExp_Properties
 */
var regExpProperties = ["$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9",
  "$_", "$*", "$&", "$+", "$`", "$'",
  "input", "lastMatch", "lastParen", "leftContext", "rightContext"
];

var regExpPropertiesDefaultValues = (function () {
  var values = Object.create(null);
  regExpProperties.forEach(function (property) {
    values[property] = RegExp[property];
  });
  return values;
}());


/**
 * Tests that executing the provided function (which may use regular expressions
 * in its implementation) does not create or modify unwanted properties on the
 * RegExp constructor.
 */
function testForUnwantedRegExpChanges(testFunc) {
  regExpProperties.forEach(function (property) {
    RegExp[property] = regExpPropertiesDefaultValues[property];
  });
  testFunc();
  regExpProperties.forEach(function (property) {
    if (RegExp[property] !== regExpPropertiesDefaultValues[property]) {
      $ERROR("RegExp has unexpected property " + property + " with value " +
        RegExp[property] + ".");
    }
  });
}


/**
 * Tests whether name is a valid BCP 47 numbering system name
 * and not excluded from use in the ECMAScript Internationalization API.
 * @param {string} name the name to be tested.
 * @return {boolean} whether name is a valid BCP 47 numbering system name and
 *   allowed for use in the ECMAScript Internationalization API.
 */

function isValidNumberingSystem(name) {

  // source: CLDR file common/bcp47/number.xml; version CLDR 36.1.
  var numberingSystems = [
    "adlm",
    "ahom",
    "arab",
    "arabext",
    "armn",
    "armnlow",
    "bali",
    "beng",
    "bhks",
    "brah",
    "cakm",
    "cham",
    "cyrl",
    "deva",
    "diak",
    "ethi",
    "finance",
    "fullwide",
    "geor",
    "gong",
    "gonm",
    "grek",
    "greklow",
    "gujr",
    "guru",
    "hanidays",
    "hanidec",
    "hans",
    "hansfin",
    "hant",
    "hantfin",
    "hebr",
    "hmng",
    "hmnp",
    "java",
    "jpan",
    "jpanfin",
    "jpanyear",
    "kali",
    "khmr",
    "knda",
    "lana",
    "lanatham",
    "laoo",
    "latn",
    "lepc",
    "limb",
    "mathbold",
    "mathdbl",
    "mathmono",
    "mathsanb",
    "mathsans",
    "mlym",
    "modi",
    "mong",
    "mroo",
    "mtei",
    "mymr",
    "mymrshan",
    "mymrtlng",
    "native",
    "newa",
    "nkoo",
    "olck",
    "orya",
    "osma",
    "rohg",
    "roman",
    "romanlow",
    "saur",
    "shrd",
    "sind",
    "sinh",
    "sora",
    "sund",
    "takr",
    "talu",
    "taml",
    "tamldec",
    "telu",
    "thai",
    "tirh",
    "tibt",
    "traditio",
    "vaii",
    "wara",
    "wcho",
  ];

  var excluded = [
    "finance",
    "native",
    "traditio"
  ];


  return numberingSystems.indexOf(name) !== -1 && excluded.indexOf(name) === -1;
}


/**
 * Provides the digits of numbering systems with simple digit mappings,
 * as specified in 11.3.2.
 */

var numberingSystemDigits = {
  adlm: "𞥐𞥑𞥒𞥓𞥔𞥕𞥖𞥗𞥘𞥙",
  ahom: "𑜰𑜱𑜲𑜳𑜴𑜵𑜶𑜷𑜸𑜹",
  arab: "٠١٢٣٤٥٦٧٨٩",
  arabext: "۰۱۲۳۴۵۶۷۸۹",
  bali: "\u1B50\u1B51\u1B52\u1B53\u1B54\u1B55\u1B56\u1B57\u1B58\u1B59",
  beng: "০১২৩৪৫৬৭৮৯",
  bhks: "𑱐𑱑𑱒𑱓𑱔𑱕𑱖𑱗𑱘𑱙",
  brah: "𑁦𑁧𑁨𑁩𑁪𑁫𑁬𑁭𑁮𑁯",
  cakm: "𑄶𑄷𑄸𑄹𑄺𑄻𑄼𑄽𑄾𑄿",
  cham: "꩐꩑꩒꩓꩔꩕꩖꩗꩘꩙",
  deva: "०१२३४५६७८९",
  diak: "𑥐𑥑𑥒𑥓𑥔𑥕𑥖𑥗𑥘𑥙",
  fullwide: "０１２３４５６７８９",
  gong: "𑶠𑶡𑶢𑶣𑶤𑶥𑶦𑶧𑶨𑶩",
  gonm: "𑵐𑵑𑵒𑵓𑵔𑵕𑵖𑵗𑵘𑵙",
  gujr: "૦૧૨૩૪૫૬૭૮૯",
  guru: "੦੧੨੩੪੫੬੭੮੯",
  hanidec: "〇一二三四五六七八九",
  hmng: "𖭐𖭑𖭒𖭓𖭔𖭕𖭖𖭗𖭘𖭙",
  hmnp: "𞅀𞅁𞅂𞅃𞅄𞅅𞅆𞅇𞅈𞅉",
  java: "꧐꧑꧒꧓꧔꧕꧖꧗꧘꧙",
  kali: "꤀꤁꤂꤃꤄꤅꤆꤇꤈꤉",
  khmr: "០១២៣៤៥៦៧៨៩",
  knda: "೦೧೨೩೪೫೬೭೮೯",
  lana: "᪀᪁᪂᪃᪄᪅᪆᪇᪈᪉",
  lanatham: "᪐᪑᪒᪓᪔᪕᪖᪗᪘᪙",
  laoo: "໐໑໒໓໔໕໖໗໘໙",
  latn: "0123456789",
  lepc: "᱀᱁᱂᱃᱄᱅᱆᱇᱈᱉",
  limb: "\u1946\u1947\u1948\u1949\u194A\u194B\u194C\u194D\u194E\u194F",
  mathbold: "𝟎𝟏𝟐𝟑𝟒𝟓𝟔𝟕𝟖𝟗",
  mathdbl: "𝟘𝟙𝟚𝟛𝟜𝟝𝟞𝟟𝟠𝟡",
  mathmono: "𝟶𝟷𝟸𝟹𝟺𝟻𝟼𝟽𝟾𝟿",
  mathsanb: "𝟬𝟭𝟮𝟯𝟰𝟱𝟲𝟳𝟴𝟵",
  mathsans: "𝟢𝟣𝟤𝟥𝟦𝟧𝟨𝟩𝟪𝟫",
  mlym: "൦൧൨൩൪൫൬൭൮൯",
  modi: "𑙐𑙑𑙒𑙓𑙔𑙕𑙖𑙗𑙘𑙙",
  mong: "᠐᠑᠒᠓᠔᠕᠖᠗᠘᠙",
  mroo: "𖩠𖩡𖩢𖩣𖩤𖩥𖩦𖩧𖩨𖩩",
  mtei: "꯰꯱꯲꯳꯴꯵꯶꯷꯸꯹",
  mymr: "၀၁၂၃၄၅၆၇၈၉",
  mymrshan: "႐႑႒႓႔႕႖႗႘႙",
  mymrtlng: "꧰꧱꧲꧳꧴꧵꧶꧷꧸꧹",
  newa: "𑑐𑑑𑑒𑑓𑑔𑑕𑑖𑑗𑑘𑑙",
  nkoo: "߀߁߂߃߄߅߆߇߈߉",
  olck: "᱐᱑᱒᱓᱔᱕᱖᱗᱘᱙",
  orya: "୦୧୨୩୪୫୬୭୮୯",
  osma: "𐒠𐒡𐒢𐒣𐒤𐒥𐒦𐒧𐒨𐒩",
  rohg: "𐴰𐴱𐴲𐴳𐴴𐴵𐴶𐴷𐴸𐴹",
  saur: "꣐꣑꣒꣓꣔꣕꣖꣗꣘꣙",
  segment: "🯰🯱🯲🯳🯴🯵🯶🯷🯸🯹",
  shrd: "𑇐𑇑𑇒𑇓𑇔𑇕𑇖𑇗𑇘𑇙",
  sind: "𑋰𑋱𑋲𑋳𑋴𑋵𑋶𑋷𑋸𑋹",
  sinh: "෦෧෨෩෪෫෬෭෮෯",
  sora: "𑃰𑃱𑃲𑃳𑃴𑃵𑃶𑃷𑃸𑃹",
  sund: "᮰᮱᮲᮳᮴᮵᮶᮷᮸᮹",
  takr: "𑛀𑛁𑛂𑛃𑛄𑛅𑛆𑛇𑛈𑛉",
  talu: "᧐᧑᧒᧓᧔᧕᧖᧗᧘᧙",
  tamldec: "௦௧௨௩௪௫௬௭௮௯",
  telu: "౦౧౨౩౪౫౬౭౮౯",
  thai: "๐๑๒๓๔๕๖๗๘๙",
  tibt: "༠༡༢༣༤༥༦༧༨༩",
  tirh: "𑓐𑓑𑓒𑓓𑓔𑓕𑓖𑓗𑓘𑓙",
  vaii: "꘠꘡꘢꘣꘤꘥꘦꘧꘨꘩",
  wara: "𑣠𑣡𑣢𑣣𑣤𑣥𑣦𑣧𑣨𑣩",
  wcho: "𞋰𞋱𞋲𞋳𞋴𞋵𞋶𞋷𞋸𞋹",
};


/**
 * Tests that number formatting is handled correctly. The function checks that the
 * digit sequences in formatted output are as specified, converted to the
 * selected numbering system, and embedded in consistent localized patterns.
 * @param {Array} locales the locales to be tested.
 * @param {Array} numberingSystems the numbering systems to be tested.
 * @param {Object} options the options to pass to Intl.NumberFormat. Options
 *   must include {useGrouping: false}, and must cause 1.1 to be formatted
 *   pre- and post-decimal digits.
 * @param {Object} testData maps input data (in ES5 9.3.1 format) to expected output strings
 *   in unlocalized format with Western digits.
 */

function testNumberFormat(locales, numberingSystems, options, testData) {
  locales.forEach(function (locale) {
    numberingSystems.forEach(function (numbering) {
      var digits = numberingSystemDigits[numbering];
      var format = new Intl.NumberFormat([locale + "-u-nu-" + numbering], options);

      function getPatternParts(positive) {
        var n = positive ? 1.1 : -1.1;
        var formatted = format.format(n);
        var oneoneRE = "([^" + digits + "]*)[" + digits + "]+([^" + digits + "]+)[" + digits + "]+([^" + digits + "]*)";
        var match = formatted.match(new RegExp(oneoneRE));
        if (match === null) {
          $ERROR("Unexpected formatted " + n + " for " +
            format.resolvedOptions().locale + " and options " +
            JSON.stringify(options) + ": " + formatted);
        }
        return match;
      }

      function toNumbering(raw) {
        return raw.replace(/[0-9]/g, function (digit) {
          return digits[digit.charCodeAt(0) - "0".charCodeAt(0)];
        });
      }

      function buildExpected(raw, patternParts) {
        var period = raw.indexOf(".");
        if (period === -1) {
          return patternParts[1] + toNumbering(raw) + patternParts[3];
        } else {
          return patternParts[1] +
            toNumbering(raw.substring(0, period)) +
            patternParts[2] +
            toNumbering(raw.substring(period + 1)) +
            patternParts[3];
        }
      }

      if (format.resolvedOptions().numberingSystem === numbering) {
        // figure out prefixes, infixes, suffixes for positive and negative values
        var posPatternParts = getPatternParts(true);
        var negPatternParts = getPatternParts(false);

        Object.getOwnPropertyNames(testData).forEach(function (input) {
          var rawExpected = testData[input];
          var patternParts;
          if (rawExpected[0] === "-") {
            patternParts = negPatternParts;
            rawExpected = rawExpected.substring(1);
          } else {
            patternParts = posPatternParts;
          }
          var expected = buildExpected(rawExpected, patternParts);
          var actual = format.format(input);
          if (actual !== expected) {
            $ERROR("Formatted value for " + input + ", " +
            format.resolvedOptions().locale + " and options " +
            JSON.stringify(options) + " is " + actual + "; expected " + expected + ".");
          }
        });
      }
    });
  });
}


/**
 * Return the components of date-time formats.
 * @return {Array} an array with all date-time components.
 */

function getDateTimeComponents() {
  return ["weekday", "era", "year", "month", "day", "hour", "minute", "second", "timeZoneName"];
}


/**
 * Return the valid values for the given date-time component, as specified
 * by the table in section 12.1.1.
 * @param {string} component a date-time component.
 * @return {Array} an array with the valid values for the component.
 */

function getDateTimeComponentValues(component) {

  var components = {
    weekday: ["narrow", "short", "long"],
    era: ["narrow", "short", "long"],
    year: ["2-digit", "numeric"],
    month: ["2-digit", "numeric", "narrow", "short", "long"],
    day: ["2-digit", "numeric"],
    hour: ["2-digit", "numeric"],
    minute: ["2-digit", "numeric"],
    second: ["2-digit", "numeric"],
    timeZoneName: ["short", "long"]
  };

  var result = components[component];
  if (result === undefined) {
    $ERROR("Internal error: No values defined for date-time component " + component + ".");
  }
  return result;
}


/**
 * @description Tests whether timeZone is a String value representing a
 * structurally valid and canonicalized time zone name, as defined in
 * sections 6.4.1 and 6.4.2 of the ECMAScript Internationalization API
 * Specification.
 * @param {String} timeZone the string to be tested.
 * @result {Boolean} whether the test succeeded.
 */

function isCanonicalizedStructurallyValidTimeZoneName(timeZone) {
  /**
   * Regular expression defining IANA Time Zone names.
   *
   * Spec: IANA Time Zone Database, Theory file
   */
  var fileNameComponent = "(?:[A-Za-z_]|\\.(?!\\.?(?:/|$)))[A-Za-z.\\-_]{0,13}";
  var fileName = fileNameComponent + "(?:/" + fileNameComponent + ")*";
  var etcName = "(?:Etc/)?GMT[+-]\\d{1,2}";
  var systemVName = "SystemV/[A-Z]{3}\\d{1,2}(?:[A-Z]{3})?";
  var legacyName = etcName + "|" + systemVName + "|CST6CDT|EST5EDT|MST7MDT|PST8PDT|NZ";
  var zoneNamePattern = new RegExp("^(?:" + fileName + "|" + legacyName + ")$");

  if (typeof timeZone !== "string") {
    return false;
  }
  // 6.4.2 CanonicalizeTimeZoneName (timeZone), step 3
  if (timeZone === "UTC") {
    return true;
  }
  // 6.4.2 CanonicalizeTimeZoneName (timeZone), step 3
  if (timeZone === "Etc/UTC" || timeZone === "Etc/GMT") {
    return false;
  }
  return zoneNamePattern.test(timeZone);
}
