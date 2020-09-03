package com.facebook.hermes.intl;

import android.icu.text.NumberFormat;
import android.icu.util.ULocale;
import android.os.Build;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class PlatformNumberFormatter {


    // This is an expensive implementation.. Must be optimized.
    public static List<String> filterLocales (List<String> requestedLocales, String matcher) throws JSRangeErrorException {
        ArrayList<String> supportedLocales = new ArrayList<>();
        for (String candidateLocale : requestedLocales) {
            ArrayList<String> canonicalCandidateLocaleList = new ArrayList<>();
            canonicalCandidateLocaleList.add(candidateLocale);

            LocaleMatcher2.LocaleMatchResult localeMatchResult;
            java.util.Locale[] availableLocalesArray;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                availableLocalesArray = android.icu.text.NumberFormat.getAvailableLocales();

            } else {
                availableLocalesArray = java.text.NumberFormat.getAvailableLocales();
            }

            localeMatchResult = (new LocaleMatcher2()).match(canonicalCandidateLocaleList.toArray(new String[canonicalCandidateLocaleList.size()]), availableLocalesArray, matcher);

            if(localeMatchResult.matchedLocale != null && !localeMatchResult.isDefault) {
                supportedLocales.add(localeMatchResult.matchedRequestedLocale.toCanonicalTag());
            }
        }

        return supportedLocales;
    }

    public static PlatformCollator.LocaleResolutionResult resolveLocales(List<String> locales, String localeMatcher) throws JSRangeErrorException {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            // Wierdly, the method returns an array of java.util.Locale objects .. not icu ULocale array.
            Locale[] availableLocalesArray = android.icu.text.NumberFormat.getAvailableLocales();

            LocaleMatcher2.LocaleMatchResult localeMatchResult = (new LocaleMatcher2()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // Note that we copy only collation extension as we know that "collation" is the only relevant keyword for icu.RuleBasedCollator;
            // ; as returned by RuleBasedCollator.getKeywords()
            // TODO: Spec requires & to be future proof, this should be dynamic.
            // copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        } else {
            java.util.Locale[] availableLocalesArray = android.icu.text.NumberFormat.getAvailableLocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<java.util.Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        }
    }
}
