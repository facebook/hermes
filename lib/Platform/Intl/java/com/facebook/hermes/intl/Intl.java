/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Intl {

    private static boolean isAlphaNum(String name, int min, int max) {
        char[] chars = name.toCharArray();

        if (chars.length < min || chars.length > max) {
            return false;
        }

        for (char c : chars) {
            if (!Character.isLetter(c) && !Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isAlpha(String name, int min, int max) {
        char[] chars = name.toCharArray();

        if (chars.length < min || chars.length > max) {
            return false;
        }

        for (char c : chars) {
            if (!Character.isLetter(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isDigit(String name, int min, int max) {
        char[] chars = name.toCharArray();

        if (chars.length < min || chars.length > max) {
            return false;
        }

        for (char c : chars) {
            if (!Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isDigitAlphanum3(String token) {
        return token.length() == 4 && Character.isDigit(token.charAt(0)) && isAlphaNum(token.substring(1), 3, 3);
    }

    private static boolean isUnicodeLanguageSubtag(String token) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{2,3} | alpha{5,8};
        return isAlpha(token, 2, 3) || isAlpha(token, 5, 8);
    }

    private static boolean isExtensionSingleton(String token) {
        return isAlphaNum(token, 1, 1);
    }

    private static boolean isUnicodeScriptSubtag(String token) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{4};
        return isAlpha(token, 4, 4);
    }

    private static boolean isUnicodeRegionSubtag(String token) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        //= (alpha{2} | digit{3}) ;
        return isAlpha(token, 2, 2) || isDigit(token, 3, 3);
    }

    private static boolean isUnicodeVariantSubtag(String token) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (alphanum{5,8}
        // | digit alphanum{3}) ;
        return isAlphaNum(token, 5, 8) || isDigitAlphanum3(token);
    }


    // https://tc39.es/ecma402/#sec-isstructurallyvalidlanguagetag
    // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
    // Notes:: This is a hightly compromised implementation of the specification.
    private static boolean isStructurallyValidLanguageTag(String languageTag) {
        String subTagSepatatorRegex = "-|_";
        String[] tokens = languageTag.split(subTagSepatatorRegex);

        if (tokens.length == 0)
            return false;

        if (!isUnicodeLanguageSubtag(tokens[0]))
            return false;

        if (tokens.length == 1)
            return true;

        // https://en.wikipedia.org/wiki/IETF_language_tag#Extensions
        // We don;t bother the rest of the tags.
        if (isExtensionSingleton(tokens[1]))
            return true;

        int index = 1;
        if (isUnicodeScriptSubtag(tokens[index])) {
            index++;
            if (index == tokens.length)
                return true;
        }

        if (isUnicodeRegionSubtag(tokens[index])) {
            index++;
        }

        while (index < tokens.length) {
            if (isExtensionSingleton(tokens[index]))
                return true;

            if (!isUnicodeVariantSubtag(tokens[index]))
                return false;

            index++;
        }

        // Note: We are skiping the following as they can be expensive and doesn't add much value.
        // Other engines (V8, Chakra) also skip these validations as of today.
        // * does not include duplicate variant subtags, and *
        // * does not include duplicate singleton subtags. *

        return true;
    }

    private static String canonicalizeUnicodeLocaleIdUsingPlatformICU(String locale) {
        // A quick comparative study with other implementations.
        // The canonical way to canonicalize a localeId string is to roundtrip it through the icu::Locale object using forLanguageTag/toLanguageTag functions.
        // V8 relies on icu4c implementation of the above functions, but augmented with private tables and code for handling special cases and error scenarios
        // https://github.com/v8/v8/blob/4b9b23521e6fd42373ebbcb20ebe03bf445494f9/src/objects/intl-objects.cc
        // Also, note that Chromium applies a few patches (https://chromium.googlesource.com/chromium/deps/icu/+/refs/heads/master/patches/) over icu, which may also result in subtle behaviour differences.
        //
        // Firefox doesn't seem to rely on ICU much but custom implemented most code and tables
        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/Locale.cpp#1233
        // Firefox has intentionally reimplemented them for performance as documented here:
        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/LanguageTag.cpp#1034
        //
        // Chakra official releases links to Windows.Globalization libraries available in Windows,
        // But has an ICU variant which roundtrips the localId through icu::Locale as mentioned before with no other custom code around.
        //
        // Note:: icu4j is not a JNI wrapper around icu4c, but a reimplementation using Java.
        // Even though they have similar APIs, there are subtle deviations for e.g. in error handling.
        // Unlike the icu4c equivalents, the forLanguageTag in icu4j doesn't report any errors while parsing ..
        // For e.g. icu4c identifies bogus locids.. and report failure when part of the loclid can't be parsed. (https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4c/source/common/locid.cpp#L816)
        // This results in our implementation a bit more leniant (i.e. not throwing RangeError for certain invalid inputs) campared to V8/Chakra
        // Both icu4c and icu4j implementations use private tables and mappings embedded in code for special cases such as deprecated and grandfathered  language code.
        // icu4c: https://github.com/unicode-org/icu/blob/9219c6ae038a3556dffca880e93d2f2ca00e685a/icu4c/source/common/uloc_tag.cpp#L103
        // icu4j: https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4j/main/classes/core/src/com/ibm/icu/impl/locale/LanguageTag.java#L43
        // Android-icu4j: https://android.googlesource.com/platform/external/icu/+/refs/heads/master/android_icu4j/
        //
        // Clearly icu4c implementation has a few more private tables implementated, for e.g. the deprecated languages and regions. This results in subtle behavioural differences between our implemenation and say, V8.
        //
        // As of now, Firefox seems to be by far the most compliant engine out there.
        //
        // Our current implementation is directly over icu4j available with Android platform, as hasn't implemented any private tables/mappings yet.
        // We fail many tests in our testcases based on https://github.com/tc39/test262/tree/master/test/intl402/Intl, but mostly special scenarios.
        // Our implementation closely follows the current V8 implementation, and our differences from V8 stems from
        // 1. The behavioural difference between icu4j and icu4c
        // 2. We didnt' implement some of the private tables and mappings other than what we get from ICU. (for e.g. https://github.com/v8/v8/blob/d432b2185ca9f2c1c405df872e5546cd2cf2dfd0/src/objects/intl-objects.cc#L802)
        // The failed test cases are commented with TODO::Hermes tag in intl_getCanonicalNames_*.js
        //
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/ULocale.html#forLanguageTag-java.lang.String-
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1Locale.html#af76028775e37fd75a30209aaede551e2

        ULocale uLocale= ULocale.forLanguageTag(locale);
        String tag = uLocale.toLanguageTag();
        return tag;

        // Note: TODO : An alternative implementation could use https://developer.android.com/reference/android/icu/util/ULocale.Builder to build the locale.. which does structural validation internally based on documentation.
    }

    // https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid
    // Definition of Canonical Unicode Locale Ids: https://unicode.org/reports/tr35/#Canonical_Unicode_Locale_Identifiers
    private static String canonicalizeUnicodeLocaleId(String locale) {
        String canonical = null;

        // We rely on platform libraries for actual transformation to the canonical form.
        //
        // Starting from Android 7.0 (API level 24), the Android platform exposes a subset of the ICU4J APIs for app developers to use under the android.icu package.
        //
        // Prior to Android 7.0, Android platform uses ICU and CLDR to implement various classes for handling both Latin and non-Latin orthographies,
        // exposing classes like Locale, Character, and many subclasses of java.text.
        // But, forLanguageTag and toLanguageTag methods in java.util.Locale got added in API Level 21
        //
        // For older devices, we take a crude fallback .. assuming that the locale id is already canonicalized.
        //
        // https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid
        // Steps 1. and 2.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            canonical = canonicalizeUnicodeLocaleIdUsingPlatformICU(locale);
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            canonical = java.util.Locale.forLanguageTag(locale).toLanguageTag();
        } else {
            canonical = locale;
        }

        assert (canonical != null);

        // 3.
        // This step ensures that a Unicode locale extension sequence in the returned language tag contains:
        //
        //    only the first instance of any attribute duplicated in the input, and
        //    only the first keyword for a given key in the input.
        // Note : TODO: We are currently skipping this step ..

        return canonical;
    }

    // Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
    private static List<String> canonicalizeLocaleList(List<String> locales) throws
            JSRangeErrorException {
        // 1. If locales is undefined, then
        // Return a new empty List.
        if (locales.size() == 0) {
            return Collections.emptyList();
        }

        // 2. Let seen be a new empty List.
        ArrayList<String> seen = new ArrayList<String>();

        // 3. If Type(locales) is String or Type(locales) is Object and locales has an [[InitializedLocale]] internal slot, then
        // 4. Else
        // We don't yet support Locale object - https://tc39.es/ecma402/#locale-objects
        // As of now, 'locales' can only be a string list/array.
        // 'O' is not a string array of locales

        // 5. Let len be ? ToLength(? Get(O, "length")).
        // 6. Let k be 0.
        // 7. Repeat, while k < len

        for (String locale : locales) {
            // We don't have steps for 7a. 7b. 7c. i-iv  .. as we only allow string arrays here..

            // Smoke validation.
            // Throw RangeError if input locale string is (1) empty (2) non-ASCII string.
            if (locale == null || locale.trim().isEmpty()) {
                throw new JSRangeErrorException("Incorrect locale information provided");
            }

            if (!Pattern.compile("\\p{ASCII}*$").matcher(locale).matches()) {
                throw new JSRangeErrorException(String.format("Invalid language tag: %s", locale));
            }

            // Normalize the string by lower casing and avoiding trailing whitespaces
            locale = locale.toLowerCase().trim();

            // 7.c.v
            if (!isStructurallyValidLanguageTag(locale))
                throw new JSRangeErrorException(String.format("Incorrect locale information provided: %s", locale==null? "null":locale));

            // 7.c.vi
            String canonicalizedTag = canonicalizeUnicodeLocaleId(locale);

            // 7.c.vii
            if (!canonicalizedTag.isEmpty() && !seen.contains(canonicalizedTag)) {
                seen.add(canonicalizedTag);
            }
        }

        return seen;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-canonicalizelocalelist
    //
    // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
    // for more discussion of locales and CanonicalizeLocaleList.
    public static List<String> getCanonicalLocales(List<String> locales)
            throws JSRangeErrorException {
        return canonicalizeLocaleList(locales);
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sup-string.prototype.tolocalelowercase
    public static String toLocaleLowerCase(List<String> locales, String str) {
        return "lowered";
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sup-string.prototype.tolocaleuppercase
    public static String toLocaleUpperCase(List<String> locales, String str) {
        return "uppered";
    }
}
