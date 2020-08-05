/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.regex.Pattern;

public class Intl {

    // Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
    private static List<String> canonicalizeLocaleList(List<String> locales) throws
            JSRangeErrorException {
        // 1. If locales is undefined, then
        // Return a new empty List.
        if (locales.size() == 0) {
            return Collections.emptyList();
        }

        // Note:: Some other major input validation occurs closer to VM in 'normalizeLocales' in JSLib/Intl.cpp

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
            if (locale == null) {
                throw new JSRangeErrorException("Incorrect locale information provided");
            }

            if (locale.isEmpty()) {
                throw new JSRangeErrorException("Incorrect locale information provided");
            }

            // 7.c.v & 7.c.vi
            String canonicalizedTag= LocaleIdentifier.canonicalizeLocaleId(locale);

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
