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

    private LocaleMatchResult bestFitMatch(String[] requestedLocales, T[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            // Note:: We are trying one candidate at a time so that we know which of the requested locales matched.
            for (String requestedLocale : requestedLocales) {
                ArrayList<ULocale> candidateLocales = new ArrayList<>();
                ILocaleObject localeObject = LocaleObjectICU4J.createFromLocaleId(requestedLocale);

                candidateLocales.add(ULocale.forLanguageTag(localeObject.toCanonicalTagWithoutExtensions()));
                ULocale[] candidateLocaleArray = candidateLocales.toArray(new ULocale[candidateLocales.size()]);

                // Note: We assume that this method does "best fit" matching, which is not yet verified.
                // Note: ICU4J's LocaleMatcher (https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/LocaleMatcher.html) is not available on Android !
                android.icu.util.ULocale acceptedLocale = android.icu.util.ULocale.acceptLanguage(candidateLocaleArray, (ULocale[]) availableLocales, null);

                if (acceptedLocale != null) {
                    result.matcher = Constants.LOCALEMATCHER_BESTFIT;
                    result.matchedLocale = LocaleObjectICU4J.createFromULocale(acceptedLocale);
                    result.matchedRequestedLocale = LocaleObjectICU4J.createFromLocaleId(requestedLocale);
                    result.isDefault = false;

                    return result;
                }
            }
        } else {
            // We don't have best fit matcher available on older platforms.
            result = lookupMatch(requestedLocales, availableLocales);
            result.matcher = Constants.LOCALEMATCHER_BESTFIT; // Lookup fit is the best fit that we have.
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
