// Copyright 2018 André Bargull; Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-intl.locale
description: >
    Verifies getters with normal tags.
info: |
    Intl.Locale.prototype.toString ()
    3. Return loc.[[Locale]].

    get Intl.Locale.prototype.baseName
    5. Return the substring of locale corresponding to the
       language ["-" script] ["-" region] *("-" variant)
       subsequence of the langtag grammar.

    get Intl.Locale.prototype.language
    4. Return the substring of locale corresponding to the language production.

    get Intl.Locale.prototype.script
    7. Return the substring of locale corresponding to the script production.

    get Intl.Locale.prototype.region
    7. Return the substring of locale corresponding to the region production.

    get Intl.Locale.prototype.calendar
    3. Return loc.[[Calendar]].

    get Intl.Locale.prototype.collation
    3. Return loc.[[Collation]].

    get Intl.Locale.prototype.hourCycle
    3. Return loc.[[HourCycle]].

    get Intl.Locale.prototype.caseFirst
    This property only exists if %Locale%.[[RelevantExtensionKeys]] contains "kf".
    3. Return loc.[[CaseFirst]].

    get Intl.Locale.prototype.numeric
    This property only exists if %Locale%.[[RelevantExtensionKeys]] contains "kn".
    3. Return loc.[[Numeric]].

    get Intl.Locale.prototype.numberingSystem
    3. Return loc.[[NumberingSystem]].

features: [Intl.Locale]
---*/

// Test all getters return the expected results.
var langtag = "de-latn-de-u-ca-gregory-co-phonebk-hc-h23-kf-true-kn-false-nu-latn";
var loc = new Intl.Locale(langtag);

assert.sameValue(loc.toString(), "de-Latn-DE-u-ca-gregory-co-phonebk-hc-h23-kf-kn-false-nu-latn");
assert.sameValue(loc.baseName, "de-Latn-DE");
assert.sameValue(loc.language, "de");
assert.sameValue(loc.script, "Latn");
assert.sameValue(loc.region, "DE");
assert.sameValue(loc.calendar, "gregory");
assert.sameValue(loc.collation, "phonebk");
assert.sameValue(loc.hourCycle, "h23");
if ("caseFirst" in loc) {
    assert.sameValue(loc.caseFirst, "");
}
if ("numeric" in loc) {
    assert.sameValue(loc.numeric, false);
}
assert.sameValue(loc.numberingSystem, "latn");

// Replace all components through option values and validate the getters still
// return the expected results.
var loc = new Intl.Locale(langtag, {
    language: "ja",
    script: "jpan",
    region: "jp",
    calendar: "japanese",
    collation: "search",
    hourCycle: "h24",
    caseFirst: "false",
    numeric: "true",
    numberingSystem: "jpanfin",
});

assert.sameValue(loc.toString(), "ja-Jpan-JP-u-ca-japanese-co-search-hc-h24-kf-false-kn-nu-jpanfin");
assert.sameValue(loc.baseName, "ja-Jpan-JP");
assert.sameValue(loc.language, "ja");
assert.sameValue(loc.script, "Jpan");
assert.sameValue(loc.region, "JP");
assert.sameValue(loc.calendar, "japanese");
assert.sameValue(loc.collation, "search");
assert.sameValue(loc.hourCycle, "h24");
if ("caseFirst" in loc) {
    assert.sameValue(loc.caseFirst, "false");
}
if ("numeric" in loc) {
    assert.sameValue(loc.numeric, true);
}
assert.sameValue(loc.numberingSystem, "jpanfin");

// Replace only some components through option values and validate the getters
// return the expected results.
var loc = new Intl.Locale(langtag, {
    language: "fr",
    region: "ca",
    collation: "standard",
    hourCycle: "h11",
});

assert.sameValue(loc.toString(), "fr-Latn-CA-u-ca-gregory-co-standard-hc-h11-kf-kn-false-nu-latn");
assert.sameValue(loc.baseName, "fr-Latn-CA");
assert.sameValue(loc.language, "fr");
assert.sameValue(loc.script, "Latn");
assert.sameValue(loc.region, "CA");
assert.sameValue(loc.calendar, "gregory");
assert.sameValue(loc.collation, "standard");
assert.sameValue(loc.hourCycle, "h11");
if ("caseFirst" in loc) {
    assert.sameValue(loc.caseFirst, "");
}
if ("numeric" in loc) {
    assert.sameValue(loc.numeric, false);
}
assert.sameValue(loc.numberingSystem, "latn");
