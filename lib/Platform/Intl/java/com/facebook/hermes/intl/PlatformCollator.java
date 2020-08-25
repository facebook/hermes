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

//    public static class LocaleResolutionOptions {
//        public String localeMatcher = Constants.LOCALEMATCHER_BESTFIT;
//        public String collation = Constants.COLLATION_DEFAULT;
//        public boolean forSearch = false; // This corresponds to localeData => %Collator%.[[SearchLocaleData]] in https://tc39.es/ecma402/#sec-initializecollator
//    }

    public static class LocaleResolutionResult {
        ILocaleObject resolvedLocale = null; // Final resolved locale to be used to build the collator. This will include only the language id and the relevant extensions (only "co")
        ILocaleObject resolvedDesiredLocale = null; // Locale object for the locale id from the list of desired locales provided by the user which is accepted.
    }

    public static LocaleResolutionResult resolveLocales(List<String> locales, String localeMatcher) throws JSRangeErrorException {

        // This implementation makes the following assumptions
        // 1. The list of available locales returned by <locale|collator>.getAvailableULocales()
        // 1.a. don't contain extensions (In other words, for all available locales, all variants and extension are supported.)
        // 1.b. are canonicalized .. i.e. <langsubtag in lower case>-<script in title case>-<region in capital>

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<ULocale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            LocaleResolutionResult result = new LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // Add the extensions .. for e.g. the "de-u-co-phonebk" ..
            ArrayList<String> collationExtensions = localeMatchResult.matchedRequestedLocale.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
            if (collationExtensions != null && collationExtensions.size() > 0) {

                ArrayList<String> resolvedExtensions = new ArrayList<>();
                for (String collationType : collationExtensions) {
                    resolvedExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(collationType));
                }

                result.resolvedLocale.setUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_SHORT, resolvedExtensions);
            }

            return result;
        } else {
            java.util.Locale[] availableLocalesArray = java.text.RuleBasedCollator.getAvailableLocales();
            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            LocaleResolutionResult result = new LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // Add the extensions .. for e.g. the "de-u-co-phonebk" ..
            ArrayList<String> collationExtensions = localeMatchResult.matchedRequestedLocale.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
            if (collationExtensions != null && collationExtensions.size() > 0) {

                ArrayList<String> resolvedExtensions = new ArrayList<>();
                for (String collationType : collationExtensions) {
                    resolvedExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(collationType));
                }

                result.resolvedLocale.setUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_SHORT, resolvedExtensions);
            }

            return result;
        }



//            for (String locale : locales) {
//                ArrayList<android.icu.util.ULocale> candidateLocales = new ArrayList<>();
//                ILocaleObject localeObject = LocaleObjectICU4J.createFromLocaleId(locale);
//
//                // Note :: android.icu.util.ULocale.acceptLanguage doesn't match the locale if the candidate local has unicode extensions
//                // for e.g. "android.icu.util.ULocale.acceptLanguage(new ULocale[]{ULocale.forLanguageTag("de-u-co-phonebk")}, availableLocalesArray, null)" return null;
//                candidateLocales.add(ULocale.forLanguageTag(localeObject.toCanonicalTagWithoutExtensions()));
//
//                ULocale[] candidateLocaleArray = candidateLocales.toArray(new ULocale[candidateLocales.size()]);
//
//                android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();
//                android.icu.util.ULocale acceptedLocale = android.icu.util.ULocale.acceptLanguage(candidateLocaleArray, availableLocalesArray, null);
//
//                if (acceptedLocale != null) {
//                    LocaleResolutionResult result = new LocaleResolutionResult();
//
//                    // Add relevant extensions back .. specifically if the desired locale contains "co" extension.
//
//                    result.resolvedLocale = LocaleObjectICU4J.createFromULocale(acceptedLocale);
//
//                    result.resolvedDesiredLocale = LocaleObjectICU4J.createFromLocaleId(locale);


//                    // Note:: We can't find any other way to force the collator to use search locale data .. This is what other engines such as "V8" also does .
//                    // TODO :: We need to make sure that this won't be shown in the resolvedLocales
//                    if (options.forSearch) {
//                        ArrayList<String> currentCollationExtensions = result.resolvedDesiredLocale.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
//
//                        ArrayList<String> currentResolvedCollationExtensions = new ArrayList<>();
//                        for (String currentCollationExtensio : currentCollationExtensions) {
//                            currentResolvedCollationExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(currentCollationExtensio));
//                        }
//
//                        currentResolvedCollationExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(Constants.SEARCH));
//                        result.resolvedLocale.setUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_SHORT, currentResolvedCollationExtensions);
//                    }
//
//                    return result;
//                }
//            }
//
//            if (fallbackToDefault) {
//                LocaleResolutionResult result = new LocaleResolutionResult();
//                result.resolvedLocale = LocaleObjectICU4J.createDefault();
//                result.resolvedDesiredLocale = result.resolvedLocale;
//
//                return result;
//            } else {
//                return null;
//            }
//        } else {
//
//            ArrayList<java.util.Locale> candidateLocales = new ArrayList<>();
//            for (String locale : locales) {
//                ILocaleObject localeObject = LocaleObjectAndroid.createFromLocaleId(locale);
//                candidateLocales.add((Locale) localeObject.getLocale());
//            }
//
//            java.util.Locale[] availableLocales = java.text.RuleBasedCollator.getAvailableLocales();
//
//            for (java.util.Locale candidateLocale : candidateLocales) {
//                for (java.util.Locale availableLocale : availableLocales) {
//                    if (availableLocale == candidateLocale) {
//                        LocaleResolutionResult result = new LocaleResolutionResult();
//                        result.resolvedLocale = LocaleObjectAndroid.createFromLocale(candidateLocale);
//                        result.resolvedDesiredLocale = result.resolvedLocale;
//                        return result;
//                    }
//                }
//            }
//
//            if (fallbackToDefault) {
//                LocaleResolutionResult result = new LocaleResolutionResult();
//                result.resolvedLocale = LocaleObjectAndroid.createDefault();
//                result.resolvedDesiredLocale = result.resolvedLocale;
//                return result;
//            } else {
//                return null;
//            }
//        }
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
