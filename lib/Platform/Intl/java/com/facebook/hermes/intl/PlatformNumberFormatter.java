package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;

public class PlatformNumberFormatter {

    // This is an expensive implementation.. Must be optimized.
    public static List<String> filterLocales (List<String> requestedLocales, String matcher) throws JSRangeErrorException {
        ArrayList<String> supportedLocales = new ArrayList<>();
        for (String candidateLocale : requestedLocales) {
            ArrayList<String> canonicalCandidateLocaleList = new ArrayList<>();
            canonicalCandidateLocaleList.add(candidateLocale);

            LocaleMatcher.LocaleMatchResult localeMatchResult;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                android.icu.util.ULocale[] availableLocalesArray = ULocale.getAvailableLocales();
                localeMatchResult = (new LocaleMatcher<ULocale>()).match(canonicalCandidateLocaleList.toArray(new String[canonicalCandidateLocaleList.size()]), availableLocalesArray, matcher);
            } else {
                java.util.Locale[] availableLocalesArray = java.text.NumberFormat.getAvailableLocales();
                localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(canonicalCandidateLocaleList.toArray(new String[canonicalCandidateLocaleList.size()]), availableLocalesArray, matcher);
            }

            if(localeMatchResult.matchedLocale != null && !localeMatchResult.isDefault) {
                supportedLocales.add(localeMatchResult.matchedRequestedLocale.toCanonicalTag());
            }
        }

        return supportedLocales;
    }

    public static PlatformCollator.LocaleResolutionResult resolveLocales(List<String> locales, String localeMatcher) throws JSRangeErrorException {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            android.icu.util.ULocale[] availableLocalesArray = android.icu.text.RuleBasedCollator.getAvailableULocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<ULocale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            return result;
        } else {
            java.util.Locale[] availableLocalesArray = android.icu.text.NumberFormat.getAvailableLocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            return result;
        }
    }
}
