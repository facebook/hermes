package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

public class LocaleMatcher {

    private static LocaleObject[] getAvailableLocales() {
        ArrayList<LocaleObject> availableLocalesAsPlatformLocale = new ArrayList<>();

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            for (ULocale available: ULocale.getAvailableLocales()) {
                availableLocalesAsPlatformLocale.add(LocaleObject.constructFromICU4jLocale(available));
            }
        } else {
            for (Locale available: Locale.getAvailableLocales()) {
                availableLocalesAsPlatformLocale.add(LocaleObject.constructFromLegacyLocale(available));
            }
        }

        return availableLocalesAsPlatformLocale.toArray(new LocaleObject[availableLocalesAsPlatformLocale.size()]);
    }

    // Note that the following methods are quite expensive !
    public static List<LocaleObject> filterAgainstAvailableLocales(List<String> candidateLocales) throws JSRangeErrorException {

        if(candidateLocales.size() == 0)
            return new ArrayList<>();

        ArrayList<LocaleObject> filteredLocaleIds = new ArrayList<>();

        Set<LocaleObject> candiateLocaleSet = new HashSet<>();
        for (String candidateLocaleStr: candidateLocales) {
            candiateLocaleSet.add(LocaleObject.constructFromLocaleId(candidateLocaleStr, false));
        }

        LocaleObject[] availableLocales = getAvailableLocales();

        for (LocaleObject availableLocale : availableLocales) {
            if(candiateLocaleSet.isEmpty()) // Candidates exhausted.
                break;

            for (LocaleObject candicateLocale : candiateLocaleSet) {

                if (availableLocale.matches(candicateLocale)) {
                    filteredLocaleIds.add(candicateLocale);

                    candiateLocaleSet.remove(availableLocale);
                }
            }
        }

        return filteredLocaleIds;
    }

    public static LocaleObject lookupAgainstAvailableLocales(List<String> candidateLocales, String localeMatcher) throws JSRangeErrorException {

        // Note: Currently we ignore "localeMatcher" argument, as we support only "lookup" algorithm
        // As far as i've observed, the ULocale.acceptLanguage method performs a "best fit" match. And i haven't found an API yet, which can do a simpler lookup.
        // java.util.Locale object provides APIs for the "lookup" and "filter" algorithm implemenations on top of ICU4j's LocaleMatcher.. but they got added in API 26.
        // Unfortunately, ICU4j LocaleMatcher (<https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/LocaleMatcher.html>) is not exposed in Android !


        // TODO : This implementation is currently very expensive and we could simplify it very likely.
        // For e.g. we may try with stripping the extensions always.
        // Or we could avoid this locale filtering alltogether and directly create the collator object, leave the details to it.
        // This option gives us full control and flexibility though.
        // Needs to be revisited !

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            ArrayList<ULocale> uLocales = new ArrayList<>();
            for (String candidateLocaleStr : candidateLocales) {
                LocaleObject localeObject = LocaleObject.constructFromLocaleId(candidateLocaleStr, false);
                uLocales.add(localeObject.getICU4jLocale());
            }

            boolean fallback[] = new boolean[1];
            ULocale accepted = ULocale.acceptLanguage(uLocales.toArray(new ULocale[uLocales.size()]), fallback);
            if (accepted != null) {
//                //Put the collation extension keys back
//                ULocale.Builder builder = new ULocale.Builder();
//                builder.setLocale(accepted);
//                builder.setExtension('u', "co-phonebk-kf-lower");
//                ULocale newLocale = builder.build();

                return LocaleObject.constructFromICU4jLocale(accepted);
            } else {
                // Retry after stripping extensions

                ArrayList<ULocale> uLocalesStipped = new ArrayList<>();
                for (String candidateLocaleStr : candidateLocales) {
                    LocaleObject localeObject = LocaleObject.constructFromLocaleId(candidateLocaleStr, true);
                    uLocalesStipped.add(localeObject.getICU4jLocale());
                }

                // TODO: If uLocalesStipped && uLocales are identical, don't search again.

                boolean fallback2[] = new boolean[1];
                ULocale accepted2 = ULocale.acceptLanguage(uLocalesStipped.toArray(new ULocale[uLocalesStipped.size()]), fallback2);

                if (accepted2 != null) {
                    return LocaleObject.constructFromICU4jLocale(accepted2);
                } else {
                    throw new JSRangeErrorException("Unknown locale requested !");
                }
            }
        } else {
            // Note: This code is untested !
            Set<LocaleObject> candiateLocaleSet = new HashSet<>();
            for (String candidateLocaleStr : candidateLocales) {
                candiateLocaleSet.add(LocaleObject.constructFromLocaleId(candidateLocaleStr, false));
            }

            LocaleObject[] availableLocales = getAvailableLocales();

            for (LocaleObject candicateLocale : candiateLocaleSet) {
                for (LocaleObject availableLocale : availableLocales) {

                    // This does an absolute match where every subtags should match character by character.
                    if (availableLocale.equals(candicateLocale)) {
                        return candicateLocale;
                    }
                }
            }

            throw new JSRangeErrorException("Unknown locale requested !");

        }
    }
}