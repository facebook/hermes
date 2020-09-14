package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

public class LocaleMatcher {

    public static class LocaleMatchResult {
        public ILocaleObject matchedLocale;
        public HashMap<String, String> extensions = new HashMap<>();
    }

    public static android.icu.util.ULocale[] getULocaleArrayFromLocaleArrray(java.util.Locale[] localeArray) {
        ArrayList<android.icu.util.ULocale> result = new ArrayList<>();
        for(java.util.Locale locale : localeArray) {
            result.add(ULocale.forLanguageTag(locale.toLanguageTag())); // Note: Locale.toLanguageTag() is introduced in API 21.
        }

        return result.toArray(new android.icu.util.ULocale[result.size()]);
    }

    // https://tc39.es/ecma402/#sec-bestavailablelocale
    public static String BestAvailableLocale(String[] availableLocales, String locale) {
        String candidate = locale;
        while (true) {
            // TODO:: Method take String list as argument to avoid conversions between array and list?
            // TODO:: AvailableLocales list always seems to be sorted. Is it guaranteed so that we can binary search int it ? Can we cheaply ensure that it is sorted ? Do we do multiple searches to justify sorting ?
            if (Arrays.asList(availableLocales).indexOf(candidate) > -1)
                return candidate;

            int pos = candidate.lastIndexOf("-");
            if (pos < 0)
                return ""; // We treat empty string as "undefined"

            if (pos >= 2 && candidate.charAt(pos - 2) == '-') // This is very likely unnecessary as this function is called after removing extensions.
                pos -= 2;

            candidate = candidate.substring(0, pos);
        }
    }

    public static String BestAvailableLocale(java.util.Locale[] availableLocales, String locale) {
        ArrayList<String> availableLocaleIds = new ArrayList<>();
        for (java.util.Locale availableLocale: availableLocales) {
            availableLocaleIds.add(availableLocale.toLanguageTag());
        }

        return BestAvailableLocale(availableLocaleIds.toArray(new String[availableLocaleIds.size()]), locale);
    }

    // https://tc39.es/ecma402/#sec-lookupmatcher
    public static LocaleMatchResult lookupMatch(String[] requestedLocales, String[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();
        for (String locale : requestedLocales) {
            ILocaleObject requestedLocaleObject = LocaleObject.createFromLocaleId(locale);
            String noExtensionLocale = requestedLocaleObject.toCanonicalTagWithoutExtensions();

            String availableLocale = BestAvailableLocale(availableLocales, noExtensionLocale);
            if (!availableLocale.isEmpty()) {
                result.matchedLocale = LocaleObject.createFromLocaleId(availableLocale);
                result.extensions = requestedLocaleObject.getUnicodeExtensions();
                return result;
            }
        }

        result.matchedLocale = LocaleObject.createDefault();
        return result;
    }

    public static ULocale bestFitBestAvailableLocale(String requestedLocale, android.icu.util.ULocale[] availableLocales) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);

        ILocaleObject requestedLocaleObject = LocaleObject.createFromLocaleId(requestedLocale);
        return bestFitBestAvailableLocale(requestedLocaleObject, availableLocales);
    }

    public static ULocale bestFitBestAvailableLocale(ILocaleObject requestedLocaleObject, android.icu.util.ULocale[] availableLocales) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);

        android.icu.util.ULocale requestedULocaleWithoutExtensions = (android.icu.util.ULocale) requestedLocaleObject.getLocaleWithoutExtensions();
        android.icu.util.ULocale[] requestedLocalesArray = new android.icu.util.ULocale[]{(android.icu.util.ULocale) requestedULocaleWithoutExtensions};
        boolean[] fallback = new boolean[1];

        // Note: We assume that this method does "best fit" matching, which is not yet verified.
        // Note: Based on documentation, it is a thin wrapper over LocaleMatcher (https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/LocaleMatcher.html) class which is not yet available on Android !
        android.icu.util.ULocale acceptedLocale = android.icu.util.ULocale.acceptLanguage(requestedLocalesArray, availableLocales, fallback);

        // Process if there is a match without fallback to ROOT
        if (!fallback[0] && acceptedLocale != null) {
            return acceptedLocale;
        }

        return null;
    }

    public static LocaleMatchResult bestFitMatch(String[] requestedLocales, android.icu.util.ULocale[] availableLocales) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        LocaleMatchResult result = new LocaleMatchResult();
        for (String requestedLocale : requestedLocales) {
            ILocaleObject requestedLocaleObject = LocaleObject.createFromLocaleId(requestedLocale);
            ULocale availableLocale = bestFitBestAvailableLocale(requestedLocaleObject, availableLocales);
            if(availableLocale != null) {
                result.matchedLocale = LocaleObjectICU.createFromULocale(availableLocale);
                result.extensions = requestedLocaleObject.getUnicodeExtensions();
                return result;
            }
        }

        result.matchedLocale = LocaleObjectICU.createDefault();
        return result;
    }

    // https://tc39.es/ecma402/#sec-lookupsupportedlocales
    public static String[] lookupSupportedLocales(String[] availableLocales, String[] requestedLocales) throws JSRangeErrorException {
        ArrayList<String> subset = new ArrayList<>();
        for (String requestedLocale : requestedLocales) {
            String noExtensionLocale = LocaleObject.createFromLocaleId(requestedLocale).toCanonicalTagWithoutExtensions();
            String availableLocale = BestAvailableLocale(availableLocales, noExtensionLocale);
            if(availableLocale != null && !availableLocale.isEmpty())
                subset.add(requestedLocale);
        }

        return subset.toArray(new String[subset.size()]);
    }
}
