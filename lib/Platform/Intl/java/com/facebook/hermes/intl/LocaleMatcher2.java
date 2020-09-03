package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.Locale;

public class LocaleMatcher2 {
    public class LocaleMatchResult {
        public ILocaleObject matchedLocale;
        public ILocaleObject matchedRequestedLocale;
        public String matcher;
        public boolean isDefault;
    }

    public LocaleMatchResult match (String[] requestedLocales, Locale[] availableLocales, String matcher) throws JSRangeErrorException {
        if(matcher.equals(Constants.LOCALEMATCHER_BESTFIT))
            return bestFitMatch(requestedLocales, availableLocales);
        else if (matcher.equals(Constants.LOCALEMATCHER_LOOKUP))
            return lookupMatch(requestedLocales, availableLocales);
        else
            throw new JSRangeErrorException("Unrecognized locale matcher");
    }

    private LocaleMatchResult bestFitMatch(String[] requestedLocales, Locale[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();

        // We don't have best fit matcher available on older platforms.
        result = lookupMatch(requestedLocales, availableLocales);
        result.matcher = Constants.LOCALEMATCHER_BESTFIT; // Lookup fit is the best fit that we have.
        return result;
    }

    private LocaleMatchResult lookupMatch(String[] requestedLocales, Locale[] availableLocales) throws JSRangeErrorException {

        LocaleMatchResult result = new LocaleMatchResult();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            ArrayList<String> availableLocaleTags = new ArrayList<>();
            for (Locale availableLocale: availableLocales)
                availableLocaleTags.add((availableLocale).toLanguageTag());

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
            for (Locale availableLocale: availableLocales)
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
