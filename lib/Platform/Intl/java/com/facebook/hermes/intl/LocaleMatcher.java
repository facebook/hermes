package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

public class LocaleMatcher {

    // Note: We are wrapping the list for code brevity, but this can be expensive as the available locale list is typically large (~750 on my emulator)
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

    public static List<LocaleObject> filterAgainstAvailableLocales(List<String> candidateLocales) throws JSRangeErrorException {

        if(candidateLocales.size() == 0)
            return new ArrayList<>();

        ArrayList<LocaleObject> filteredLocaleIds = new ArrayList<>();

        Set<LocaleObject> candiateLocaleSet = new HashSet<>();
        for (String candidateLocaleStr: candidateLocales) {
            candiateLocaleSet.add(LocaleObject.constructFromLocaleId(candidateLocaleStr));
        }

        for (LocaleObject availableLocale : getAvailableLocales()) {
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


        Set<LocaleObject> candiateLocaleSet = new HashSet<>();
        for (String candidateLocaleStr: candidateLocales) {
            candiateLocaleSet.add(LocaleObject.constructFromLocaleId(candidateLocaleStr));
        }

        for (LocaleObject candicateLocale : candiateLocaleSet) {
            for (LocaleObject availableLocale : getAvailableLocales()) {

                if (availableLocale.matches(candicateLocale)) {
                    return candicateLocale;
                }
            }
        }

        return null;
    }
}