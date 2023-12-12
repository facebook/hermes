/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import androidx.annotation.RequiresApi;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

public class LocaleMatcher {

  public static class LocaleMatchResult {
    public ILocaleObject<?> matchedLocale;
    public ILocaleObject<?> dataLocale = null;
    public HashMap<String, String> extensions = new HashMap<>();
  }

  // Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
  public static List<String> canonicalizeLocaleList(List<String> locales)
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

  // https://tc39.es/ecma402/#sec-bestavailablelocale
  public static String BestAvailableLocale(String[] availableLocales, String locale) {
    String candidate = locale;
    while (true) {
      // TODO:: Method take String list as argument to avoid conversions between array and list?
      // TODO:: AvailableLocales list always seems to be sorted. Is it guaranteed so that we can
      // binary search int it ? Can we cheaply ensure that it is sorted ? Do we do multiple searches
      // to justify sorting ?
      if (Arrays.asList(availableLocales).indexOf(candidate) > -1) return candidate;

      int pos = candidate.lastIndexOf("-");
      if (pos < 0) return ""; // We treat empty string as "undefined"

      if (pos >= 2
          && candidate.charAt(pos - 2)
              == '-') // This is very likely unnecessary as this function is called after removing
        // extensions.
        pos -= 2;

      candidate = candidate.substring(0, pos);
    }
  }

  // https://tc39.es/ecma402/#sec-lookupmatcher
  public static LocaleMatchResult lookupMatch(String[] requestedLocales, String[] availableLocales)
      throws JSRangeErrorException {

    LocaleMatchResult result = new LocaleMatchResult();
    for (String locale : requestedLocales) {
      ILocaleObject<?> requestedLocaleObject = LocaleObject.createFromLocaleId(locale);
      String noExtensionLocale = requestedLocaleObject.toCanonicalTagWithoutExtensions();

      String availableLocale = BestAvailableLocale(availableLocales, noExtensionLocale);
      if (!availableLocale.isEmpty()) {
        result.matchedLocale = LocaleObject.createFromLocaleId(availableLocale);
        result.dataLocale = requestedLocaleObject;
        result.extensions = requestedLocaleObject.getUnicodeExtensions();
        return result;
      }
    }

    result.matchedLocale = LocaleObject.createDefault();
    result.dataLocale = LocaleObject.createDefault();
    return result;
  }

  public static String[] getAvailableLocales() {

    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
      // Before L, Locale.toLanguageTag isn't available. Need to figure out how to get a locale id
      // from locale object ... Currently resoring to support only en
      return new String[] {"en"};
    }

    ArrayList<String> availableLocaleIds = new ArrayList<>();
    java.util.Locale[] availableLocales = java.util.Locale.getAvailableLocales();
    for (java.util.Locale locale : availableLocales) {
      availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
    }

    String[] availableLocaleIdsArray = new String[availableLocaleIds.size()];
    return availableLocaleIds.toArray(availableLocaleIdsArray);
  }

  // https://tc39.es/ecma402/#sec-lookupmatcher
  public static LocaleMatchResult lookupMatch(String[] requestedLocales)
      throws JSRangeErrorException {

    String[] availableLocales = getAvailableLocales();
    return lookupMatch(requestedLocales, availableLocales);
  }

  // https://tc39.es/ecma402/#sec-lookupsupportedlocales
  public static String[] lookupSupportedLocales(String[] requestedLocales)
      throws JSRangeErrorException {
    ArrayList<String> subset = new ArrayList<>();
    String[] availableLocales = getAvailableLocales();

    for (String requestedLocale : requestedLocales) {
      String noExtensionLocale =
          LocaleObject.createFromLocaleId(requestedLocale).toCanonicalTagWithoutExtensions();
      String availableLocale = BestAvailableLocale(availableLocales, noExtensionLocale);
      if (availableLocale != null && !availableLocale.isEmpty()) subset.add(requestedLocale);
    }

    String[] subsetArray = new String[subset.size()];
    return subset.toArray(subsetArray);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public static ULocale bestFitBestAvailableLocale(ILocaleObject<?> requestedLocaleObject)
      throws JSRangeErrorException {
    ULocale[] availableLocales = ULocale.getAvailableLocales();

    android.icu.util.ULocale requestedULocaleWithoutExtensions =
        (android.icu.util.ULocale) requestedLocaleObject.getLocaleWithoutExtensions();
    android.icu.util.ULocale[] requestedLocalesArray =
        new android.icu.util.ULocale[] {requestedULocaleWithoutExtensions};
    boolean[] fallback = new boolean[1];

    android.icu.util.ULocale acceptedLocale =
        android.icu.util.ULocale.acceptLanguage(requestedLocalesArray, availableLocales, fallback);

    // Process if there is a match without fallback to ROOT
    if (!fallback[0] && acceptedLocale != null) {
      return acceptedLocale;
    }

    return null;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public static LocaleMatchResult bestFitMatch(String[] requestedLocales)
      throws JSRangeErrorException {
    LocaleMatchResult result = new LocaleMatchResult();
    for (String requestedLocale : requestedLocales) {
      ILocaleObject<?> requestedLocaleObject = LocaleObject.createFromLocaleId(requestedLocale);
      ULocale availableLocale = bestFitBestAvailableLocale(requestedLocaleObject);
      if (availableLocale != null) {
        result.matchedLocale = LocaleObjectICU.createFromULocale(availableLocale);
        result.dataLocale = requestedLocaleObject;
        result.extensions = requestedLocaleObject.getUnicodeExtensions();
        return result;
      }
    }

    result.matchedLocale = LocaleObjectICU.createDefault();
    result.dataLocale = LocaleObjectICU.createDefault();
    return result;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public static String[] bestFitSupportedLocales(String[] requestedLocales)
      throws JSRangeErrorException {
    ArrayList<String> subset = new ArrayList<>();
    for (String requestedLocale : requestedLocales) {
      ULocale uLocale =
          bestFitBestAvailableLocale(LocaleObject.createFromLocaleId(requestedLocale));
      if (uLocale != null) subset.add(requestedLocale);
    }

    String[] subsetArray = new String[subset.size()];
    return subset.toArray(subsetArray);
  }
}
