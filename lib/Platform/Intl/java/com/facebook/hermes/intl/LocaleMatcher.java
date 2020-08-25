package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.Locale;

public class LocaleMatcher <T> {
    public class LocaleMatchResult {
        public ILocaleObject matchedLocale;
        public ILocaleObject matchedRequestedLocale;
        public String matcher;
        public boolean isDefault;
    }

    public LocaleMatchResult match (String[] requestedLocales, T[] availableLocales, String matcher) throws JSRangeErrorException {
        if(matcher.equals(Constants.LOCALEMATCHER_BESTFIT))
            return bestFitMatch(requestedLocales, availableLocales);
        else if (matcher.equals(Constants.LOCALEMATCHER_LOOKUP))
            return lookupMatch(requestedLocales, availableLocales);
        else
            throw new JSRangeErrorException("Unrecognized locale matcher");
    }

    private ILocaleObject getLocaleObject(String localeId) throws JSRangeErrorException {
        return LocaleObjectICU4J.createFromLocaleId(localeId);
    }

    private LocaleMatchResult bestFitMatch(String[] requestedLocales, T[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            for (String requestedLocale : requestedLocales) {
                ArrayList<ULocale> candidateLocales = new ArrayList<>();
                ILocaleObject localeObject = LocaleObjectICU4J.createFromLocaleId(requestedLocale);

                // Note :: android.icu.util.ULocale.acceptLanguage doesn't match the locale if the candidate local has unicode extensions
                // for e.g. "android.icu.util.ULocale.acceptLanguage(new ULocale[]{ULocale.forLanguageTag("de-u-co-phonebk")}, availableLocalesArray, null)" return null;
                candidateLocales.add(ULocale.forLanguageTag(localeObject.toCanonicalTagWithoutExtensions()));

                ULocale[] candidateLocaleArray = candidateLocales.toArray(new ULocale[candidateLocales.size()]);

                android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();

                // Note: We assume that this method doesn "best fit" matching, which is not yet verified.
                android.icu.util.ULocale acceptedLocale = android.icu.util.ULocale.acceptLanguage(candidateLocaleArray, availableLocalesArray, null);

                if (acceptedLocale != null) {
                    // Add relevant extensions back .. specifically if the desired locale contains "co" extension.

                    result.matcher = Constants.LOCALEMATCHER_BESTFIT;
                    result.matchedLocale = LocaleObjectICU4J.createFromULocale(acceptedLocale);
                    result.matchedRequestedLocale = LocaleObjectICU4J.createFromLocaleId(requestedLocale);
                    result.isDefault = false;

                    return result;

//                    // Add the extensions .. for e.g. the "de-u-co-phonebk" ..
//                    ArrayList<String> collationExtensions = result.resolvedDesiredLocale.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
//                    if (collationExtensions != null && collationExtensions.size() > 0) {
//
//                        ArrayList<String> resolvedExtensions = new ArrayList<>();
//                        for (String collationType : collationExtensions) {
//                            resolvedExtensions.add(UnicodeLocaleKeywordUtils.resolveCollationKeyword(collationType));
//                        }
//
//                        result.resolvedLocale.setUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_SHORT, resolvedExtensions);
//                    }
//
//                    // for e.g. "es_TRADITIONAL" locale uses traditional sorting order which is different form the default modern sorting of spanish.
//                    ArrayList<String> desiredLocaleVariants = result.resolvedDesiredLocale.getVariants();
//                    if (desiredLocaleVariants != null && desiredLocaleVariants.size() > 0) {
//                        result.resolvedLocale.setVariant(desiredLocaleVariants);
//                    }
//
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

                }
            }
        } else {
            // We don't have best fit matcher available on older platforms.
            result = lookupMatch(requestedLocales, availableLocales);
            result.matcher = Constants.LOCALEMATCHER_BESTFIT;
            return result;
        }

        result.matcher = Constants.LOCALEMATCHER_BESTFIT;
        result.matchedLocale = LocaleObjectICU4J.createDefault();
        result.matchedRequestedLocale = result.matchedLocale;
        result.isDefault = true;

        return result;
    }

    private LocaleMatchResult lookupMatch(String[] requestedLocales, T[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            ArrayList<String> availableLocaleTags = new ArrayList<>();
            for (T availableLocale: availableLocales)
                availableLocaleTags.add(((ULocale)availableLocale).toLanguageTag());

            for (String locale : requestedLocales) {
                ILocaleObject requestedLocaleObject = LocaleObjectICU4J.createFromLocaleId(locale);
                String requestedLocaleCanonicalTag = requestedLocaleObject.toCanonicalTagWithoutExtensions();

                while (!requestedLocaleCanonicalTag.isEmpty()) {
                    int idx = availableLocaleTags.indexOf(requestedLocaleCanonicalTag);
                    if(idx > -1) {
                        result.matcher = Constants.LOCALEMATCHER_LOOKUP;
                        result.matchedLocale = LocaleObjectICU4J.createFromLocaleId(requestedLocaleCanonicalTag);
                        result.matchedRequestedLocale = requestedLocaleObject;
                        result.isDefault = false;
                        return result;
                    }

                    int lastSepIndex = requestedLocaleCanonicalTag.lastIndexOf("-");
                    if(lastSepIndex > -1)
                        requestedLocaleCanonicalTag = requestedLocaleCanonicalTag.substring(0, lastSepIndex);
                    else
                        break;
                }
            }

            result.matcher = Constants.LOCALEMATCHER_LOOKUP;
            result.matchedLocale = LocaleObjectICU4J.createDefault();
            result.matchedRequestedLocale = result.matchedLocale;
            result.isDefault = true;

            return result;

        } else {

            ArrayList<String> availableLocaleTags = new ArrayList<>();
            for (T availableLocale: availableLocales)
                availableLocaleTags.add(((Locale)availableLocale).toLanguageTag());

            for (String locale : requestedLocales) {
                ILocaleObject requestedLocaleObject = LocaleObjectAndroid.createFromLocaleId(locale);
                String requestedLocaleCanonicalTag = requestedLocaleObject.toCanonicalTagWithoutExtensions();

                while (!requestedLocaleCanonicalTag.isEmpty()) {
                    int idx = availableLocaleTags.indexOf(requestedLocaleCanonicalTag);
                    if(idx > -1) {
                        result.matcher = Constants.LOCALEMATCHER_LOOKUP;
                        result.matchedLocale = LocaleObjectAndroid.createFromLocaleId(requestedLocaleCanonicalTag);
                        result.matchedRequestedLocale = requestedLocaleObject;
                        result.isDefault = false;
                        return result;
                    }

                    int lastSepIndex = requestedLocaleCanonicalTag.lastIndexOf("-");
                    if(lastSepIndex > -1)
                        requestedLocaleCanonicalTag = requestedLocaleCanonicalTag.substring(0, lastSepIndex);
                    else
                        break;
                }
            }

            result.matcher = Constants.LOCALEMATCHER_LOOKUP;
            result.matchedLocale = LocaleObjectAndroid.createDefault();
            result.matchedRequestedLocale = result.matchedLocale;
            result.isDefault = true;

            return result;
        }
    }
}
