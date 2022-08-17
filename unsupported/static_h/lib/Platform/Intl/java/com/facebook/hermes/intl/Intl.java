/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.os.Build;
import com.facebook.proguard.annotations.DoNotStrip;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

@DoNotStrip
public class Intl {

  // Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
  private static List<String> canonicalizeLocaleList(List<String> locales)
      throws JSRangeErrorException {
    // 1. If locales is undefined, then
    // Return a new empty List.
    if (locales.size() == 0) {
      return Collections.emptyList();
    }

    // Note:: Some other major input validation occurs closer to VM in 'normalizeLocales' in
    // JSLib/Intl.cpp

    // 2. Let seen be a new empty List.
    ArrayList<String> seen = new ArrayList<>();

    // 3. If Type(locales) is String or Type(locales) is Object and locales has an
    // [[InitializedLocale]] internal slot, then
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
      String canonicalizedTag = LocaleIdentifier.canonicalizeLocaleId(locale);

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
  @DoNotStrip
  public static List<String> getCanonicalLocales(List<String> locales)
      throws JSRangeErrorException {
    return canonicalizeLocaleList(locales);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sup-string.prototype.tolocalelowercase
  @DoNotStrip
  public static String toLocaleLowerCase(List<String> locales, String str)
      throws JSRangeErrorException {
    String[] localesArray = new String[locales.size()];
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
      LocaleMatcher.LocaleMatchResult localeMatchResult =
          LocaleMatcher.bestFitMatch(locales.toArray(localesArray));
      return android.icu.lang.UCharacter.toLowerCase(
          (android.icu.util.ULocale) localeMatchResult.matchedLocale.getLocale(), str);
    } else {
      LocaleMatcher.LocaleMatchResult localeMatchResult =
          LocaleMatcher.lookupMatch(locales.toArray(localesArray));
      return str.toLowerCase((java.util.Locale) localeMatchResult.matchedLocale.getLocale());
    }
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sup-string.prototype.tolocaleuppercase
  @DoNotStrip
  public static String toLocaleUpperCase(List<String> locales, String str)
      throws JSRangeErrorException {
    String[] localesArray = new String[locales.size()];
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
      LocaleMatcher.LocaleMatchResult localeMatchResult =
          LocaleMatcher.bestFitMatch(locales.toArray(localesArray));
      return android.icu.lang.UCharacter.toUpperCase(
          (android.icu.util.ULocale) localeMatchResult.matchedLocale.getLocale(), str);
    } else {
      LocaleMatcher.LocaleMatchResult localeMatchResult =
          LocaleMatcher.lookupMatch(locales.toArray(localesArray));
      return str.toUpperCase((java.util.Locale) localeMatchResult.matchedLocale.getLocale());
    }
  }
}
