package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class PlatformCollator {

    public static IPlatformCollator createFromLocale(ILocaleObject localeObject) throws JSRangeErrorException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return PlatformCollatorICU4J.createFromLocale(localeObject);
        } else {
            return PlatformCollatorAndroid.createFromLocale(localeObject);
        }
    }

    public static String[] getRelevantExtensionKeys() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return android.icu.text.RuleBasedCollator.getKeywords();
        } else {
            return new String[0];
        }
    }

    public static String[] getRelevantExtensionKeysForLocale(String key, ILocaleObject locale) throws JSRangeErrorException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return android.icu.text.RuleBasedCollator.getKeywordValuesForLocale(key, (ULocale) locale.getLocale(), false);
        } else {
            return new String[0];
        }
    }

    public static ArrayList<String> getAvailableLocales() throws JSRangeErrorException {
        ArrayList<String> availableLocales = new ArrayList<>();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            for (android.icu.util.ULocale availableLocale : android.icu.text.RuleBasedCollator.getAvailableULocales())
                availableLocales.add(availableLocale.toLanguageTag());
        } else {
            for (java.util.Locale availableLocale : java.text.RuleBasedCollator.getAvailableLocales())
                availableLocales.add(availableLocale.toLanguageTag());
        }

        return availableLocales;
    }

    public static class LocaleResolutionResult {
        ILocaleObject resolvedLocale = null; // Final resolved locale to be used to build the collator. This will include only the language id and the relevant extensions (only "co")
        ILocaleObject resolvedDesiredLocale = null; // Locale object for the locale id from the list of desired locales provided by the user which is accepted.
    }

    private static void copyCollationExtension (ILocaleObject source, ILocaleObject target) throws JSRangeErrorException {
        // Add relevant extensions back .. specifically if the desired locale contains "co" extension.
        ArrayList<String> collationExtensions = source.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
        if (collationExtensions != null && collationExtensions.size() > 0) {

            ArrayList<String> resolvedExtensions = new ArrayList<>();
            for (String collationType : collationExtensions) {
                // https://tc39.es/ecma402/#sec-intl-collator-internal-slots
                if(collationType.equals(Constants.COLLATION_SEARCH) || collationType.equals(Constants.COLLATION_STANDARD))
                    continue;

                resolvedExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(collationType));
            }

            target.setUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_SHORT, resolvedExtensions);
        }
    }

    public static List<String> filterLocales (List<String> requestedLocales, String matcher) throws JSRangeErrorException {
        ArrayList<String> supportedLocales = new ArrayList<>();
        for (String candidateLocale : requestedLocales) {
            ArrayList<String> canonicalCandidateLocaleList = new ArrayList<>();
            canonicalCandidateLocaleList.add(candidateLocale);

            LocaleMatcher.LocaleMatchResult localeMatchResult;
            // TODO :: We need to refactor this to avoid platform checks ..
            // Note : "getAvailableLocales" call is expensive and we should try to reuse the results ..
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();
                localeMatchResult = (new LocaleMatcher<ULocale>()).match(requestedLocales.toArray(new String[requestedLocales.size()]), availableLocalesArray, matcher);
            } else {
                java.util.Locale[] availableLocalesArray = java.text.RuleBasedCollator.getAvailableLocales();
                localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(requestedLocales.toArray(new String[requestedLocales.size()]), availableLocalesArray, matcher);
            }

            if(localeMatchResult.matchedLocale != null && !localeMatchResult.isDefault) {
                supportedLocales.add(localeMatchResult.matchedRequestedLocale.toCanonicalTag());
            }
        }

        return supportedLocales;
    }

    public static LocaleResolutionResult resolveLocales(List<String> locales, String localeMatcher) throws JSRangeErrorException {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<ULocale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            LocaleResolutionResult result = new LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        } else {
            java.util.Locale[] availableLocalesArray = java.text.RuleBasedCollator.getAvailableLocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            LocaleResolutionResult result = new LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        }
    }

    public static boolean isIgnorePunctuationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isNumericCollationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean isCaseFirstCollationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

}
