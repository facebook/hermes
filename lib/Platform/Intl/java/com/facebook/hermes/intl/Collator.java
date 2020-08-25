/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
//
//class PlatformCollatorObject {
//    private android.icu.text.RuleBasedCollator icu4jCollator = null;
//    private java.text.RuleBasedCollator legacyCollator = null;
//
//    private PlatformCollatorObject(ILocaleObject locale) {
//
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            icu4jCollator = (RuleBasedCollator) RuleBasedCollator.getInstance(locale.);
//
//            // Normalization is always on by the spec. We don't know whether the text is already normalized.
//            // This has perf implications.
//            icu4jCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);
//
//        } else {
//            legacyCollator = (java.text.RuleBasedCollator) java.text.Collator.getInstance(locale.getLegacyLocale());
//            legacyCollator.setDecomposition(java.text.Collator.CANONICAL_DECOMPOSITION);
//        }
//    }
//
//    public static PlatformCollatorObject getInstance(LocaleObject locale) {
//        return new PlatformCollatorObject(locale);
//    }
//
//
//
//    public int compare(String source, String target) {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            return icu4jCollator.compare(source, target);
//        } else {
//            return legacyCollator.compare(source, target);
//        }
//    }
//
//    public boolean isSensitiySupported(String sensitivity) {
//        // Legacy mode don't support sensitivity "case" since the collator object doesn't support "setCaseLevel" method.
//        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N && sensitivity.compareTo(Constants.SENSITIVITY_CASE) == 0) {
//            return false;
//        }
//
//        return true;
//    }
//
//    public void setSensitivity(String sensitivity) {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            switch (sensitivity) {
//                case Constants.SENSITIVITY_BASE:
//                    icu4jCollator.setStrength(android.icu.text.Collator.PRIMARY);
//                    break;
//                case Constants.SENSITIVITY_ACCENT:
//                    icu4jCollator.setStrength(android.icu.text.Collator.SECONDARY);
//                    break;
//                case Constants.SENSITIVITY_CASE:
//                    icu4jCollator.setStrength(android.icu.text.Collator.PRIMARY);
//                    icu4jCollator.setCaseLevel(true);
//                    break;
//                case Constants.SENSITIVITY_VARIANT:
//                    icu4jCollator.setStrength(android.icu.text.Collator.TERTIARY);
//                    break;
//            }
//        } else {
//            switch (sensitivity) {
//                case Constants.SENSITIVITY_BASE:
//                    legacyCollator.setStrength(android.icu.text.Collator.PRIMARY);
//                    break;
//                case Constants.SENSITIVITY_ACCENT:
//                    legacyCollator.setStrength(android.icu.text.Collator.SECONDARY);
//                    break;
//                case Constants.SENSITIVITY_CASE:
//                    throw new UnsupportedOperationException("Unsupported Sensitivity option is Collator");
//                case Constants.SENSITIVITY_VARIANT:
//                    legacyCollator.setStrength(android.icu.text.Collator.TERTIARY);
//                    break;
//            }
//        }
//    }
//
//    public void setIgnorePunctuation(boolean ignore) {
//        // TODO:: According to documentation, it should take effect only when the strength is se to "QUATERNARY". Need to test it.
//        if (ignore && (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N))
//            icu4jCollator.setAlternateHandlingShifted(true);
//    }
//
//    public static boolean isIgnorePunctuationSupported() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            return true;
//        } else {
//            return false;
//        }
//    }
//
//    public static boolean isNumericCollationSupported() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            return true;
//        } else {
//            return false;
//        }
//    }
//
//    public void setNumericAttribute(boolean numeric) {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            icu4jCollator.setNumericCollation(numeric);
//        } else {
//            throw new UnsupportedOperationException("Numeric collation not supported !");
//        }
//    }
//
//    public static boolean isCaseFirstCollationSupported() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            return true;
//        } else {
//            return false;
//        }
//    }
//
//    public void setCaseFirstAttribute(String caseFirst) {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
//            switch (caseFirst) {
//                case "upper":
//                    icu4jCollator.setUpperCaseFirst(true);
//                    break;
//
//                case "lower":
//                    icu4jCollator.setLowerCaseFirst(true);
//                    break;
//
//                case "false":
//                default:
//                    icu4jCollator.setCaseFirstDefault();
//                    break;
//            }
//        } else {
//            throw new UnsupportedOperationException("CaseFirst collation attribute not supported !");
//        }
//    }
//}

/**
 * This class represents the Java part of the Android Intl.Collator
 * implementation.  The interaction with the Hermes JaveScript
 * internals are implemented in C++ and should not generally need to
 * be changed.  Implementers' notes here will describe what parts of
 * the ECMA 402 spec remain to be implemented.
 * <p>
 * Also see the implementer' notes on DateTimeFormat.java.
 */
public class Collator {

    // [[RelevantExtensionKeys]]
    // Ref: https://tc39.es/ecma402/#sec-intl.locale-internal-slots
    // According to ecma401, for locale objects, The value of the [[RelevantExtensionKeys]] internal slot is « "ca", "co", "hc", "kf", "kn", "nu" ».
    // https://tc39.es/ecma402/#sec-intl-collator-internal-slots
    // And, for collator object, The value of the [[RelevantExtensionKeys]] internal slot is a List that must include the element "co", may include any or all of the elements "kf" and "kn",
    // There doesn't seem to exist a clean way to map [[RelevantExtensionKeys]] with ICU.
    // ICU collator's support keywords ("android.icu.text.RuleBasedCollator.getKeywords()) which currently support (only) one "collation", which maps cleanly to the unicode extension "co"
    // ICU collator currently doesn't support the other two ("kf", "kn") as keywords/extensions .. but there exists methods "setNumericCollation", "setUpperCaseFirst" etc. which has same functionality.
    //
    // Hence, in effect the value of [[RelevantExtensionKeys]] is ["co", "kf", "kn"] with our current implementation.
    // As this is a static fact, our code assumes the above instead of dynamic validation as prescribed in the pseudo code in ecma402

    // Internal slots as defined by https://tc39.es/ecma402/#sec-intl.collator
    // Let internalSlotsList be « [[InitializedCollator]], [[Locale]], [[Usage]], [[Sensitivity]], [[IgnorePunctuation]], [[Collation]], [[BoundCompare]] ».
    // TODO :: We don't support [[BoundCompare]] as of now.
    //
    // And we add [[Numeric]] and [[CaseFirst]] slots unconditionally as described above.

    // 11.1.2 2-4
    private String mResolvedUsage = null;
    private String mResolvedSensitivity = null;
    private boolean mResolvedIgnorePunctuation = false;
    private String mResolvedCollation = Constants.COLLATION_DEFAULT;

    // These two slots needs to differentiate between "unset" and "default" .. These shouldn't be part of resolvedOptions if "unset"
    // And these can be set through options as well as extensions ..
    private Boolean mResolvedNumeric = null;

    private String mResolvedCaseFirst = null;

    // [[Locale]]
    private ILocaleObject mResolvedLocaleObject = null;

    // The original locale provided by user which is matched.
    private ILocaleObject mDesiredLocaleObject = null;

    // [[InitializedCollator]]
    private IPlatformCollator mPlatformCollatorObject = null;

    private String resolveStringOption(Map<String, Object> options, String key, String[] possibleValues, String defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            String optionValue = (String) options.get(key);
            if (TextUtils.containsString(possibleValues, optionValue)) {
                return optionValue;
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionValue, key));
            }
        } else {
            return defaultValue;
        }
    }

    private boolean resolveBooleanOption(Map<String, Object> options, String key, boolean defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            // Note:: Our JSI interop layer ensures that this object is indeed a boolean
            return (boolean) options.get(key);
        } else {
            return defaultValue;
        }
    }

    private void resolveLocale(List<String> locales, PlatformCollator.LocaleResolutionOptions localeResolutionOptions) throws JSRangeErrorException {

        if (locales == null || locales.size() == 0) {
            mResolvedLocaleObject = LocaleObject.createDefault();
            mDesiredLocaleObject = mResolvedLocaleObject;
        } else {
            PlatformCollator.LocaleResolutionResult localeResolutionResult = PlatformCollator.resolveLocales(locales, localeResolutionOptions, true /*fallbackToDefault*/);
            mResolvedLocaleObject = localeResolutionResult.resolvedLocale;
            mDesiredLocaleObject = localeResolutionResult.resolvedDesiredLocale;
        }
    }

    // https://tc39.es/ecma402/#sec-initializecollator
    private void initializeCollator(List<String> locales, Map<String, Object> options) throws JSRangeErrorException {

        // 1.
        // Canonicalize Locale List
        // TODO

        // 4 & 5
        mResolvedUsage = resolveStringOption(options, Constants.COLLATION_OPTION_USAGE, Constants.COLLATOR_USAGE_POSSIBLE_VALUES, Constants.SORT);

        // We don't have 6 & 7
        // Note: We don't know a way to map the 'usage' option to "LocaleData" parameter to be used while resolving locales, with ICU.
        // With ICU, the only way to specify 'search' usage is through 'co=search' unicode extension .. (which is explicitly deprecated in ecma402, but internally we use this ! )

        // 9 & 10
        String desiredLocaleMatcher = resolveStringOption(options, Constants.LOCALEMATCHER, Constants.LOCALEMATCHER_POSSIBLE_VALUES, Constants.LOCALEMATCHER_BESTFIT);

        // 11,12,13
        if (options.containsKey(Constants.COLLATION_OPTION_NUMERIC) && PlatformCollator.isNumericCollationSupported()) {
            mResolvedNumeric = resolveBooleanOption(options, Constants.COLLATION_OPTION_NUMERIC, false);
        }

        // 14 & 15
        if (options.containsKey(Constants.COLLATION_OPTION_CASEFIRST) && PlatformCollator.isCaseFirstCollationSupported()) {
            mResolvedCaseFirst = resolveStringOption(options, Constants.COLLATION_OPTION_CASEFIRST, Constants.CASEFIRST_POSSIBLE_VALUES, Constants.CASEFIRST_FALSE);
        }

        // 8, 10, 13, 15 .. Note With our implementation, "kf" and "kb" don't influence locale selection.
        PlatformCollator.LocaleResolutionOptions localeResolutionOptions = new PlatformCollator.LocaleResolutionOptions();
        localeResolutionOptions.localeMatcher = desiredLocaleMatcher;
        localeResolutionOptions.forSearch = (mResolvedUsage.equals(Constants.SEARCH));

        // 16, 17 & 18
        // Let r be ResolveLocale(%Collator%.[[AvailableLocales]], requestedLocales, opt, relevantExtensionKeys, localeData).
        resolveLocale(locales, localeResolutionOptions);
        mPlatformCollatorObject = PlatformCollator.createFromLocale(mResolvedLocaleObject);

        // 19-21
        // If the locale id has collation keys, then they should be added to resolvedOptions.

        ArrayList<String> collationExtension = mResolvedLocaleObject.getUnicodeExtensions(Constants.COLLATION_EXTENSION_KEY_LONG);
        if (collationExtension != null && collationExtension.size() > 0)
            mResolvedCollation = collationExtension.get(0);

        // if "kf" is not yet set through options, look for extensions in the desired locale.
        if (mResolvedCaseFirst == null && PlatformCollator.isCaseFirstCollationSupported()) {
            ArrayList<String> kfExtensions = mResolvedLocaleObject.getUnicodeExtensions(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_LONG);
            if (kfExtensions != null && kfExtensions.size() > 0) {
                mResolvedCaseFirst = kfExtensions.get(0);
            }
        }

        // if "kn" is not yet set through options, look for extensions in the desired locale.
        if (mResolvedNumeric == null && PlatformCollator.isNumericCollationSupported()) {
            ArrayList<String> knExtensions = mResolvedLocaleObject.getUnicodeExtensions(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_LONG);

            // false if the "kf" is followed by a "false" .. true if not more tokens or "true"
            if (knExtensions != null && knExtensions.size() > 0 && knExtensions.get(0).equals("false"))
                mResolvedNumeric = false;
            else
                mResolvedNumeric = true;

        }

        // 21 & 22
        configureNumericCollation();

        // 23
        configureCaseFirst();


        // 24 & 25
        mResolvedSensitivity = resolveStringOption(options, Constants.COLLATION_OPTION_SENSITIVITY, Constants.SENSITIVITY_POSSIBLE_VALUES, "");
        if (mResolvedSensitivity.isEmpty() && mResolvedUsage.equals(Constants.SORT)) {
            mResolvedSensitivity = Constants.SENSITIVITY_VARIANT;

            // Note:: We don't know how to map 25.b with ICU.
        }

        // 26
        configureSensitivity();

        // 27
        mResolvedIgnorePunctuation = resolveBooleanOption(options, Constants.COLLATION_OPTION_IGNOREPUNCTUATION, false);

        // 28
        configureIgnorePunctuation();
    }

    private void configureNumericCollation() {
        if (mResolvedNumeric != null)
            mPlatformCollatorObject.setNumericAttribute(mResolvedNumeric);
    }

    private void configureCaseFirst() {
        if (mResolvedCaseFirst != null)
            mPlatformCollatorObject.setCaseFirstAttribute(mResolvedCaseFirst);
    }

    private void configureSensitivity() {
        mPlatformCollatorObject.setSensitivity(mResolvedSensitivity);
    }

    private void configureIgnorePunctuation() {
        mPlatformCollatorObject.setIgnorePunctuation(mResolvedIgnorePunctuation);
    }

    // options are usage:string, localeMatcher:string, numeric:boolean, caseFirst:string,
    // sensitivity:string, ignorePunctuation:boolean
    //
    // Implementer note: The ctor corresponds roughly to
    // https://tc39.es/ecma402/#sec-initializecollator
    // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
    public Collator(List<String> locales, Map<String, Object> options)
            throws JSRangeErrorException {

        initializeCollator(locales, options);
    }

    // options are localeMatcher:string
    //
    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-intl.collator.supportedlocalesof
    //
    // The notes on DateTimeFormat#DateTimeFormat() for Locales and
    // Options also apply here.
    public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options)
            throws JSRangeErrorException {

        ArrayList<String> supportedLocales = new ArrayList<>();
        ArrayList<String> availableLocales = PlatformCollator.getAvailableLocales();

        for (String candidateLocale : locales) {
            ArrayList<String> canonicalCandidateLocaleList = new ArrayList<>();
            canonicalCandidateLocaleList.add(candidateLocale);

            // Note :: Any locale which is accepted, even with a fallback, is added to the list as is.
            PlatformCollator.LocaleResolutionResult localeResolutionResult = PlatformCollator.resolveLocales(canonicalCandidateLocaleList, new PlatformCollator.LocaleResolutionOptions(), false);
            if (localeResolutionResult.resolvedLocale != null) {
                supportedLocales.add(localeResolutionResult.resolvedDesiredLocale.toCanonicalTag());
            }
        }

        return supportedLocales;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions
    //
    // Also see the implementer notes on DateTimeFormat#resolvedOptions()
    public Map<String, Object> resolvedOptions() throws JSRangeErrorException {
        HashMap<String, Object> finalResolvedOptions = new HashMap<>();
        finalResolvedOptions.put(Constants.LOCALE, mResolvedLocaleObject.toCanonicalTag());
        finalResolvedOptions.put(Constants.COLLATION_OPTION_USAGE, mResolvedUsage);

        if (!mResolvedSensitivity.isEmpty())
            finalResolvedOptions.put(Constants.COLLATION_OPTION_SENSITIVITY, mResolvedSensitivity);

        finalResolvedOptions.put(Constants.COLLATION_OPTION_IGNOREPUNCTUATION, mResolvedIgnorePunctuation);
        finalResolvedOptions.put(Constants.COLLATION, mResolvedCollation);

        if (mResolvedNumeric != null)
            finalResolvedOptions.put(Constants.COLLATION_OPTION_NUMERIC, mResolvedNumeric);

        if (mResolvedCaseFirst != null)
            finalResolvedOptions.put(Constants.COLLATION_OPTION_CASEFIRST, mResolvedCaseFirst);

        return finalResolvedOptions;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-collator-comparestrings
    public double compare(String source, String target) {
        return mPlatformCollatorObject.compare(source, target);
    }
}

