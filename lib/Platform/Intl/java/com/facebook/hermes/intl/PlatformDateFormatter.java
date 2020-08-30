package com.facebook.hermes.intl;

import android.os.Build;

import java.util.List;
import java.util.Locale;

public class PlatformDateFormatter {


    public static PlatformCollator.LocaleResolutionResult resolveLocales(List<String> locales, String localeMatcher) throws JSRangeErrorException {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            // Wierdly, the method returns an array of java.util.Locale objects .. not icu ULocale array.
            Locale[] availableLocalesArray = android.icu.text.DateFormat.getAvailableLocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // Note that we copy only collation extension as we know that "collation" is the only relevant keyword for icu.RuleBasedCollator;
            // ; as returned by RuleBasedCollator.getKeywords()
            // TODO: Spec requires & to be future proof, this should be dynamic.
            // copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        } else {
            Locale[] availableLocalesArray = android.icu.text.DateFormat.getAvailableLocales();

            LocaleMatcher.LocaleMatchResult localeMatchResult = (new LocaleMatcher<Locale>()).match(locales.toArray(new String[locales.size()]), availableLocalesArray, localeMatcher);

            PlatformCollator.LocaleResolutionResult result = new PlatformCollator.LocaleResolutionResult();
            result.resolvedLocale = localeMatchResult.matchedLocale;
            result.resolvedDesiredLocale = localeMatchResult.matchedRequestedLocale;

            // copyCollationExtension(result.resolvedDesiredLocale, result.resolvedLocale);

            return result;
        }
    }
}
