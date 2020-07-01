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
        // Other engines (V8, Chakra) does the same.
        // * does not include duplicate variant subtags, and *
        // * does not include duplicate singleton subtags. *

        return true;
    }

    private static String canonicalizeUnicodeLocaleIdUsingICU(String locale) {
        // Note:: Unlike the icu4c equivalents, the forLanguageTag in icu4j doesn't seem to have any error reporting ..
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/ULocale.html#forLanguageTag-java.lang.String-
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1Locale.html#af76028775e37fd75a30209aaede551e2

        ULocale uLocale= ULocale.forLanguageTag(locale);
        String tag = uLocale.toLanguageTag();
        return tag;

        // Note: TODO : An alternative implementation could use https://developer.android.com/reference/android/icu/util/ULocale.Builder to build the locale.. which does structural validation internally based on documentation.
        // But it may be more expensive because as we've already performed some structural validations which doesn't need to be repeated.
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
        // For older devices, we take a crude fallback to assume that the locale id is already canonicalized.
        // https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid
        // Steps 1. and 2.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            canonical = canonicalizeUnicodeLocaleIdUsingICU(locale);
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
