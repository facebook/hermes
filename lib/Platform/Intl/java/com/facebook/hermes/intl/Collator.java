/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.text.RuleBasedCollator;
import android.os.Build;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

class PlatformCollatorObject {
    private android.icu.text.RuleBasedCollator icu4jCollator = null;
    private java.text.RuleBasedCollator legacyCollator = null;

    private PlatformCollatorObject(LocaleObject locale) {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            icu4jCollator = (RuleBasedCollator) RuleBasedCollator.getInstance(locale.getICU4jLocale());

            // Normalization is always on by the spec. We don't know whether the text is already normalized.
            // This has perf implications.
            icu4jCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);

        } else {
            legacyCollator = (java.text.RuleBasedCollator) java.text.Collator.getInstance(locale.getLegacyLocale());
            legacyCollator.setDecomposition(java.text.Collator.CANONICAL_DECOMPOSITION);
        }
    }

    public static PlatformCollatorObject getInstance(LocaleObject locale) {
        return new PlatformCollatorObject(locale);
    }

    public static String[] getRelevantExtensionKeys() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return android.icu.text.RuleBasedCollator.getKeywords();
        } else {
            return new String[0];
        }
    }

    public static String[] getRelevantExtensionKeysForLocale(String key, LocaleObject locale) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return android.icu.text.RuleBasedCollator.getKeywordValuesForLocale(key, locale.getICU4jLocale(), false);
        } else {
            return new String[0];
        }
    }

    public int compare(String source, String target) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return icu4jCollator.compare(source, target);
        } else {
            return legacyCollator.compare(source, target);
        }
    }

    public boolean isSensitiySupported(String sensitivity) {
        // Legacy mode don't support sensitivity "case" since the collator object doesn't support "setCaseLevel" method.
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N && sensitivity.compareTo(Constants.SENSITIVITY_CASE) == 0) {
            return false;
        }

        return true;
    }

    public void setSensitivity(String sensitivity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            switch (sensitivity) {
                case Constants.SENSITIVITY_BASE:
                    icu4jCollator.setStrength(android.icu.text.Collator.PRIMARY);
                    break;
                case Constants.SENSITIVITY_ACCENT:
                    icu4jCollator.setStrength(android.icu.text.Collator.SECONDARY);
                    break;
                case Constants.SENSITIVITY_CASE:
                    icu4jCollator.setStrength(android.icu.text.Collator.PRIMARY);
                    icu4jCollator.setCaseLevel(true);
                    break;
                case Constants.SENSITIVITY_VARIANT:
                    icu4jCollator.setStrength(android.icu.text.Collator.TERTIARY);
                    break;
            }
        } else {
            switch (sensitivity) {
                case Constants.SENSITIVITY_BASE:
                    legacyCollator.setStrength(android.icu.text.Collator.PRIMARY);
                    break;
                case Constants.SENSITIVITY_ACCENT:
                    legacyCollator.setStrength(android.icu.text.Collator.SECONDARY);
                    break;
                case Constants.SENSITIVITY_CASE:
                    throw new UnsupportedOperationException("Unsupported Sensitivity option is Collator");
                case Constants.SENSITIVITY_VARIANT:
                    legacyCollator.setStrength(android.icu.text.Collator.TERTIARY);
                    break;
            }
        }
    }

    public void setIgnorePunctuation(boolean ignore) {
        // TODO:: According to documentation, it should take effect only when the strength is se to "QUATERNARY". Need to test it.
        if (ignore && (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N))
            icu4jCollator.setAlternateHandlingShifted(true);
    }

    public boolean isIgnorePunctuationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

    public boolean isNumericCollationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

    public void setNumericAttribute(boolean numeric) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            icu4jCollator.setNumericCollation(numeric);
        } else {
            throw new UnsupportedOperationException("Numeric collation not supported !");
        }
    }

    public boolean isCaseFirstCollationSupported() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return true;
        } else {
            return false;
        }
    }

    public void setCaseFirstAttribute(String caseFirst) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            switch (caseFirst) {
                case "upper":
                    icu4jCollator.setUpperCaseFirst(true);
                    break;

                case "lower":
                    icu4jCollator.setLowerCaseFirst(true);
                    break;

                case "false":
                default:
                    icu4jCollator.setCaseFirstDefault();
                    break;
            }
        } else {
            throw new UnsupportedOperationException("CaseFirst collation attribute not supported !");
        }
    }
}

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

    // This map corresponds to internal slots [[Usage]] [[Sensitivity]] [[IgnorePunctuation]] [[Collation]] [[Numeric]] and [[CaseFirst]]
    private HashMap<String, Object> resolvedOptions = null;

    // [[Locale]]
    private LocaleObject resolvedLocaleObject = null;

    // [[InitializedCollator]]
    private PlatformCollatorObject platformCollatorObject = null;

    private void resolveOptions(Map<String, Object> options) throws JSRangeErrorException {

        resolvedOptions = new HashMap<>();

        resolvedOptions.put(Constants.COLLATION, Constants.COLLATION_DEFAULT);

        if (options.containsKey(Constants.COLLATION_OPTION_USAGE)) {
            final String optionUsage = (String) options.get(Constants.COLLATION_OPTION_USAGE);
            if (Utils.containsString(Constants.COLLATOR_USAGE_POSSIBLE_VALUES, optionUsage)) {
                resolvedOptions.put(Constants.COLLATION_OPTION_USAGE, optionUsage);
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionUsage, Constants.COLLATION_OPTION_USAGE));
            }
        } else {
            resolvedOptions.put(Constants.COLLATION_OPTION_USAGE, Constants.SORT);
        }

        if (options.containsKey(Constants.LOCALEMATCHER)) {
            final String optionLocaleMatcher = (String) options.get(Constants.LOCALEMATCHER);
            if (Utils.containsString(Constants.LOCALEMATCHER_POSSIBLE_VALUES, optionLocaleMatcher)) {
                resolvedOptions.put(Constants.LOCALEMATCHER, optionLocaleMatcher);
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionLocaleMatcher, Constants.LOCALEMATCHER));
            }
        } else {
            resolvedOptions.put(Constants.LOCALEMATCHER, Constants.LOCALEMATCHER_BESTFIT);
        }

        if (options.containsKey(Constants.COLLATION_OPTION_SENSITIVITY)) {
            final String optionSensitivity = (String) options.get(Constants.COLLATION_OPTION_SENSITIVITY);
            if (Utils.containsString(Constants.SENSITIVITY_POSSIBLE_VALUES, optionSensitivity)) {
                resolvedOptions.put(Constants.COLLATION_OPTION_SENSITIVITY, optionSensitivity);
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionSensitivity, Constants.COLLATION_OPTION_SENSITIVITY));
            }
        } else {
            if (resolvedOptions.get(Constants.COLLATION_OPTION_USAGE).equals(Constants.SORT)) {
                resolvedOptions.put(Constants.COLLATION_OPTION_SENSITIVITY, Constants.SENSITIVITY_VARIANT);
            } // For usage "search", the sensitivity is locale dependant
        }

        if (options.containsKey(Constants.COLLATION_OPTION_IGNOREPUNCTUATION)) {
            boolean optionIgnorePunctuation = (boolean) options.get(Constants.COLLATION_OPTION_IGNOREPUNCTUATION);
            resolvedOptions.put(Constants.COLLATION_OPTION_IGNOREPUNCTUATION, optionIgnorePunctuation);
        } else {
            resolvedOptions.put(Constants.COLLATION_OPTION_IGNOREPUNCTUATION, false);
        }

        if (options.containsKey(Constants.COLLATION_OPTION_NUMERIC)) {
            boolean optionIsNumeric = (boolean) options.get(Constants.COLLATION_OPTION_NUMERIC);
            resolvedOptions.put(Constants.COLLATION_OPTION_NUMERIC, optionIsNumeric);
        }

        if (options.containsKey(Constants.COLLATION_OPTION_CASEFIRST)) {
            String optionCaseFirst = (String) options.get(Constants.COLLATION_OPTION_CASEFIRST);
            if (Utils.containsString(Constants.CASEFIRST_POSSIBLE_VALUES, optionCaseFirst)) {
                resolvedOptions.put(Constants.COLLATION_OPTION_CASEFIRST, optionCaseFirst);
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionCaseFirst, Constants.COLLATION_OPTION_CASEFIRST));
            }
        }
    }

    private void resolveLocale(List<String> locales, String[] relevantExtensionKeys) throws JSRangeErrorException {

        if (locales == null || locales.size() == 0) {
            resolvedLocaleObject = LocaleObject.constructDefault();
        } else {
            // TODO : Note: This is currently quite expensive..
            LocaleObject resolvedLocaleObjectBase = LocaleMatcher.lookupAgainstAvailableLocales(locales, (String) resolvedOptions.get(Constants.LOCALEMATCHER));
            resolvedLocaleObject = augmentWithUnicodeExtensions(resolvedLocaleObjectBase, relevantExtensionKeys, locales.get(0)); // We assume extension to be available only with the first locale in the requested list
        }
    }

    private void configureCollator(PlatformCollatorObject platformCollatorObject) {

        if (!platformCollatorObject.isSensitiySupported((String) resolvedOptions.get(Constants.COLLATION_OPTION_SENSITIVITY))) {
            resolvedOptions.put(Constants.COLLATION_OPTION_SENSITIVITY, Constants.SENSITIVITY_VARIANT);
        }

        // "usage" is configured while adding unicode extension by adding collation key "search" to the locale

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_SENSITIVITY))
            platformCollatorObject.setSensitivity((String) resolvedOptions.get(Constants.COLLATION_OPTION_SENSITIVITY));

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_IGNOREPUNCTUATION)) {
            if (!platformCollatorObject.isIgnorePunctuationSupported()) {
                resolvedOptions.remove(Constants.COLLATION_OPTION_IGNOREPUNCTUATION);
            } else {
                platformCollatorObject.setIgnorePunctuation((boolean) resolvedOptions.get(Constants.COLLATION_OPTION_IGNOREPUNCTUATION));
            }
        }

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_NUMERIC)) {
            if (!platformCollatorObject.isNumericCollationSupported()) {
                resolvedOptions.remove(Constants.COLLATION_OPTION_NUMERIC);
            } else {
                platformCollatorObject.setNumericAttribute((boolean) resolvedOptions.get(Constants.COLLATION_OPTION_NUMERIC));
            }
        }

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_CASEFIRST)) {
            if (!platformCollatorObject.isCaseFirstCollationSupported()) {
                resolvedOptions.remove(Constants.COLLATION_OPTION_CASEFIRST);
            } else {
                platformCollatorObject.setCaseFirstAttribute((String) resolvedOptions.get(Constants.COLLATION_OPTION_CASEFIRST));
            }
        }
    }

    private LocaleObject augmentWithUnicodeExtensions(LocaleObject resolvedLocaleObjectBase, String[] relevantExtensionKeys, String localeIdWithExtensions) throws JSRangeErrorException {

        // Return if collator doesn't support extensions
        if (relevantExtensionKeys.length == 0)
            return resolvedLocaleObjectBase;

        // Collator support "co", "kf" and "kn" extensions. Ref: https://tc39.es/ecma402/#sec-intl-collator-internal-slots

        // Note:: Various possible extensions are defined in http://cldr.unicode.org/core-spec

        ArrayList<String> attributes = new ArrayList<>();
        HashMap<String, String> keywords = new HashMap<String, String>();
        HashMap<String, String> defaults = new HashMap<String, String>();
        defaults.put(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_SHORT, "true");
        defaults.put(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_SHORT, "false");

        LocaleObject.constructFromLocaleId(localeIdWithExtensions, false /*Don't strip extensions*/).getUnicodeExtensions(attributes, keywords, defaults);

        // This is currently the only way that we know to set "search" usage with ICU .. Note that setting "search" usage through extension is against the spec.
        // Ref: https://tc39.es/ecma402/#sec-intl-collator-internal-slots
        if (resolvedOptions.get(Constants.COLLATION_OPTION_USAGE).equals(Constants.SEARCH)) {
            keywords.put(Constants.COLLATION_EXTENSION_KEY_SHORT, Constants.SEARCH);
        }

        if (keywords.isEmpty())
            return resolvedLocaleObjectBase;

        StringBuffer newUnicodeExtensionSequence = new StringBuffer();

        // ref: http://cldr.unicode.org/core-spec
        if (Utils.containsString(relevantExtensionKeys, Constants.COLLATION_EXTENSION_KEY_LONG) && keywords.containsKey(Constants.COLLATION_EXTENSION_KEY_SHORT)) {
            String[] relevantValues = PlatformCollatorObject.getRelevantExtensionKeysForLocale(Constants.COLLATION_EXTENSION_KEY_LONG, resolvedLocaleObjectBase);

            String collationTye = keywords.get(Constants.COLLATION_EXTENSION_KEY_SHORT);

            if (Utils.containsString(relevantValues, collationTye)) {

                // Note: We should force the "search" extension when the usage is "search" but still don't report it through resolvedOptions.
                // As we mentioned earlier, this is because we can't find another way to configure collator for usage "search
                // 1. collation type is not default
                // 2. and
                // 2a. collation type is not one that is forced to become default by spec (standard, search etc.)
                // or
                // 2b. "usage" in option is "search" which forces us to apply "search" collation type
                if (collationTye.compareTo(Constants.COLLATION_DEFAULT) != 0 &&
                        (!Utils.containsString(Constants.COLLATION_OVERRIDE_TO_DEFAULT_VALUES, collationTye) || resolvedOptions.get(Constants.COLLATION_OPTION_USAGE).equals(Constants.SEARCH))) {

                    if (newUnicodeExtensionSequence.length() != 0)
                        newUnicodeExtensionSequence.append('-');
                    newUnicodeExtensionSequence.append(Constants.COLLATION_EXTENSION_KEY_SHORT + "-");
                    newUnicodeExtensionSequence.append(collationTye);
                } else {
                    collationTye = Constants.COLLATION_DEFAULT;
                }

                resolvedOptions.put(Constants.COLLATION, collationTye);
            }
        }

        // ref: http://cldr.unicode.org/core-spec
        if (Utils.containsString(relevantExtensionKeys, Constants.COLLATION_EXTENSION_PARAM_NUMERIC_LONG) && keywords.containsKey(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_SHORT) && !resolvedOptions.containsKey(Constants.COLLATION_OPTION_NUMERIC)) {

            String[] relevantValues = PlatformCollatorObject.getRelevantExtensionKeysForLocale(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_LONG, resolvedLocaleObjectBase);
            String numericType = keywords.get(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_SHORT);

            if (Utils.containsString(relevantValues, numericType)) {

                if (newUnicodeExtensionSequence.length() != 0)
                    newUnicodeExtensionSequence.append('-');
                newUnicodeExtensionSequence.append(Constants.COLLATION_EXTENSION_PARAM_NUMERIC_SHORT + "-");
                newUnicodeExtensionSequence.append(numericType);

                resolvedOptions.put(Constants.COLLATION_OPTION_NUMERIC, Boolean.parseBoolean(numericType));
            }
        }

        if (Utils.containsString(relevantExtensionKeys, Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_LONG) && keywords.containsKey(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_SHORT) && !resolvedOptions.containsKey(Constants.COLLATION_OPTION_CASEFIRST)) {

            String[] relevantValues = PlatformCollatorObject.getRelevantExtensionKeysForLocale(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_LONG, resolvedLocaleObjectBase);
            String caseFirstType = keywords.get(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_SHORT);

            if (Utils.containsString(relevantValues, caseFirstType)) {

                if (newUnicodeExtensionSequence.length() != 0)
                    newUnicodeExtensionSequence.append('-');
                newUnicodeExtensionSequence.append(Constants.COLLATION_EXTENSION_PARAM_CASEFIRST_SHORT + "-");
                newUnicodeExtensionSequence.append(caseFirstType);

                resolvedOptions.put(Constants.COLLATION_OPTION_CASEFIRST, caseFirstType);
            }
        }

        if (newUnicodeExtensionSequence.length() == 0)
            return resolvedLocaleObjectBase;

        LocaleObject newLocaleObject = LocaleObject.constructByExtendingExistingObject(resolvedLocaleObjectBase, 'u', newUnicodeExtensionSequence.toString());
        return newLocaleObject;
    }

    // options are usage:string, localeMatcher:string, numeric:boolean, caseFirst:string,
    // sensitivity:string, ignorePunctuation:boolean
    //
    // Implementer note: The ctor corresponds roughly to
    // https://tc39.es/ecma402/#sec-initializecollator
    // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
    public Collator(List<String> locales, Map<String, Object> options)
            throws JSRangeErrorException {

        // Steps 2-15 in https://tc39.es/ecma402/#sec-initializecollator
        resolveOptions(options);

        // Step 16
        String[] relevantExtensionKeys = PlatformCollatorObject.getRelevantExtensionKeys();

        // Steps 1 & 17
        resolveLocale(locales, relevantExtensionKeys);

        // Step 18
        platformCollatorObject = PlatformCollatorObject.getInstance(resolvedLocaleObject);

        // Steps 19-
        configureCollator(platformCollatorObject);
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
        List<LocaleObject> supportedLocaleObjects = LocaleMatcher.filterAgainstAvailableLocales(locales);
        ArrayList<String> supportedLocaleIds = new ArrayList<>();
        for (LocaleObject supportedLocaleObject : supportedLocaleObjects)
            supportedLocaleIds.add(supportedLocaleObject.toLocaleId());

        return supportedLocaleIds;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions
    //
    // Also see the implementer notes on DateTimeFormat#resolvedOptions()
    public Map<String, Object> resolvedOptions() {
        HashMap<String, Object> finalResolvedOptions = new HashMap<>();
        finalResolvedOptions.put(Constants.LOCALE, resolvedLocaleObject.toLocaleId());

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_USAGE))
            finalResolvedOptions.put(Constants.COLLATION_OPTION_USAGE, resolvedOptions.get(Constants.COLLATION_OPTION_USAGE));

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_SENSITIVITY))
            finalResolvedOptions.put(Constants.COLLATION_OPTION_SENSITIVITY, resolvedOptions.get(Constants.COLLATION_OPTION_SENSITIVITY));

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_IGNOREPUNCTUATION))
            finalResolvedOptions.put(Constants.COLLATION_OPTION_IGNOREPUNCTUATION, resolvedOptions.get(Constants.COLLATION_OPTION_IGNOREPUNCTUATION));

        // TODO :: Currently, we are not verifying that the extensions are accepted by the ICU4j collator, which we should do.
        if (resolvedOptions.containsKey(Constants.COLLATION))
            finalResolvedOptions.put(Constants.COLLATION, resolvedOptions.get(Constants.COLLATION));

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_NUMERIC))
            finalResolvedOptions.put(Constants.COLLATION_OPTION_NUMERIC, resolvedOptions.get(Constants.COLLATION_OPTION_NUMERIC));

        if (resolvedOptions.containsKey(Constants.COLLATION_OPTION_CASEFIRST))
            finalResolvedOptions.put(Constants.COLLATION_OPTION_CASEFIRST, resolvedOptions.get(Constants.COLLATION_OPTION_CASEFIRST));

        return finalResolvedOptions;
    }

    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-collator-comparestrings
    public double compare(String source, String target) {
        return platformCollatorObject.compare(source, target);
    }
}

